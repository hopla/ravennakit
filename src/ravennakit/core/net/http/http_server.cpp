/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#include "ravennakit/core/net/http/http_server.hpp"

#include "ravennakit/core/log.hpp"

#include <boost/beast/version.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/system/result.hpp>

class rav::HttpServer::ClientSession: public std::enable_shared_from_this<ClientSession> {
  public:
    std::function<boost::beast::http::message_generator(boost::beast::http::request<boost::beast::http::string_body>)>
        on_request_callback;
    std::function<void(boost::system::error_code ec, std::string_view what)> on_error_callback;
    std::function<void()> on_close_callback;

    ClientSession() = delete;

    explicit ClientSession(boost::asio::ip::tcp::socket&& socket) : stream_(std::move(socket)) {}

    void start() {
        boost::asio::dispatch(
            stream_.get_executor(), boost::beast::bind_front_handler(&ClientSession::do_read, shared_from_this())
        );
    }

    void reset() {
        if (stream_.socket().is_open()) {
            do_close();
        }
        on_request_callback = nullptr;
        on_error_callback = nullptr;
        on_close_callback = nullptr;
    }

  private:
    boost::beast::tcp_stream stream_;
    boost::beast::flat_buffer buffer_;
    boost::beast::http::request<boost::beast::http::string_body> request_;

    void do_read() {
        // Make the request empty before reading, otherwise the operation behavior is undefined.
        request_ = {};

        stream_.expires_after(std::chrono::seconds(30));

        // Read a request
        boost::beast::http::async_read(
            stream_, buffer_, request_, boost::beast::bind_front_handler(&ClientSession::on_read, shared_from_this())
        );
    }

    void on_read(const boost::beast::error_code& ec, std::size_t bytes_transferred) {
        std::ignore = bytes_transferred;

        // This means they closed the connection
        if (ec == boost::beast::http::error::end_of_stream)
            return do_close();

        if (ec) {
            on_error_callback(ec, "read");
            return;
        }

        // Send the response
        send_response(on_request_callback(std::move(request_)));
    }

    void send_response(boost::beast::http::message_generator&& msg) {
        bool keep_alive = msg.keep_alive();

        // Write the response
        boost::beast::async_write(
            stream_, std::move(msg),
            boost::beast::bind_front_handler(&ClientSession::on_write, shared_from_this(), keep_alive)
        );
    }

    void on_write(const bool keep_alive, const boost::beast::error_code& ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec) {
            if (on_error_callback) {
                on_error_callback(ec, "write");
            }
            return;
        }

        if (!keep_alive) {
            // This means we should close the connection, usually because the response indicated the "Connection: close"
            // semantic.
            return do_close();
        }

        // Read another request
        do_read();
    }

    void do_close() {
        boost::beast::error_code ec;
        stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
        if (ec && ec != boost::beast::errc::not_connected) {
            if (on_error_callback) {
                on_error_callback(ec, "shutdown");
            }
        }
        if (on_close_callback) {
            on_close_callback();
        }
    }
};

class rav::HttpServer::Listener: public std::enable_shared_from_this<Listener> {
  public:
    std::function<void(boost::asio::ip::tcp::socket socket)> on_accept_callback;
    std::function<void(boost::system::error_code ec, std::string_view what)> on_error_callback;

    explicit Listener(boost::asio::io_context& io_context) : io_context_(io_context), acceptor_(io_context) {}

    boost::system::result<void> start(const boost::asio::ip::tcp::endpoint& endpoint) {
        boost::system::error_code ec;
        acceptor_.open(endpoint.protocol(), ec);
        if (ec) {
            return ec;
        }

        acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if (ec) {
            return ec;
        }

        acceptor_.bind(endpoint, ec);
        if (ec) {
            return ec;
        }

        acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec) {
            return ec;
        }

        do_accept();

        return {};
    }

    void reset() {
        on_accept_callback = nullptr;
        on_error_callback = nullptr;
        if (acceptor_.is_open()) {
            boost::beast::error_code ec;
            acceptor_.close(ec);
            if (ec) {
                if (on_error_callback) {
                    on_error_callback(ec, "close");
                }
            }
        }
    }

  private:
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;

    void do_accept() {
        acceptor_.async_accept(io_context_, boost::beast::bind_front_handler(&Listener::on_accept, shared_from_this()));
    }

    void on_accept(const boost::beast::error_code& ec, boost::asio::ip::tcp::socket socket) {
        if (ec) {
            if (on_error_callback) {
                on_error_callback(ec, "accept error");
            }
            return;  // To avoid infinite loop
        }

        if (on_error_callback == nullptr) {
            return;  // Acceptor was stopped, return which should lead to deallocation of this instance
        }

        on_accept_callback(std::move(socket));
        do_accept();
    }
};

rav::HttpServer::HttpServer(boost::asio::io_context& io_context) : io_context_(io_context) {}

boost::system::result<void> rav::HttpServer::start(const std::string_view host, uint16_t port) {
    if (listener_ != nullptr) {
        return boost::asio::error::already_started;
    }

    boost::system::error_code ec;
    auto addr = boost::asio::ip::make_address(host, ec);
    if (ec) {
        return ec;
    }

    auto listener = std::make_shared<Listener>(io_context_);
    listener->on_accept_callback = [this](boost::asio::ip::tcp::socket socket) {
        auto session = std::make_shared<ClientSession>(std::move(socket));
        session->on_request_callback = [](boost::beast::http::request<boost::beast::http::string_body> request
                                       ) -> boost::beast::http::message_generator {
            auto const bad_request = [&request](boost::beast::string_view why) {
                boost::beast::http::response<boost::beast::http::string_body> res {
                    boost::beast::http::status::bad_request, request.version()
                };
                res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(boost::beast::http::field::content_type, "text/html");
                res.keep_alive(request.keep_alive());
                res.body() = std::string(why);
                res.prepare_payload();
                return res;
            };
            return {bad_request("Invalid request")};
        };
        session->on_error_callback = [](const boost::system::error_code& cb_ec, std::string_view what) {
            RAV_ERROR("Client session error: {}: {}", what, cb_ec.message());
        };
        session->on_close_callback = [this, session] {
            RAV_DEBUG("Client session closed");

            for (auto it = client_sessions_.begin(); it != client_sessions_.end(); ++it) {
                if (*it == session) {
                    (*it)->reset();  // Reset inner
                    client_sessions_.erase(it);
                    break;
                }
            }
        };
        session->start();
        client_sessions_.push_back(std::move(session));
    };
    listener->on_error_callback = [](const boost::system::error_code& cb_ec, std::string_view what) {
        RAV_ERROR("Listener error: {}: {}", what, cb_ec.message());
        // TODO: Report error
    };
    const auto result = listener->start({addr, port});
    if (result.has_error()) {
        return result;
    }

    listener_ = std::move(listener);

    return {};
}

void rav::HttpServer::stop() {
    // TODO: Reset listener callback
    if (listener_) {
        listener_->reset();
        listener_.reset();
    }

    for (const auto& session : client_sessions_) {
        session->reset();
    }
}
