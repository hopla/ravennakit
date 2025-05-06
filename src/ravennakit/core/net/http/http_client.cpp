/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#include "ravennakit/core/net/http/http_client.hpp"

boost::system::result<boost::beast::http::response<boost::beast::http::basic_string_body<char>>>
rav::HttpClient::get(boost::asio::io_context& io_context, const std::string& url) {
    return request(io_context, http::verb::get, url, {});
}

boost::system::result<boost::beast::http::response<boost::beast::http::basic_string_body<char>>>
rav::HttpClient::post(boost::asio::io_context& io_context, const std::string& url, const std::string& body) {
    return request(io_context, http::verb::post, url, body);
}

boost::system::result<boost::beast::http::response<boost::beast::http::basic_string_body<char>>>
rav::HttpClient::request(
    boost::asio::io_context& io_context, http::verb method, std::string_view url, std::string_view body
) {
    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::beast::tcp_stream stream(io_context);

    auto uri_result = boost::urls::parse_uri(url);

    if (uri_result.has_error()) {
        return uri_result.error();
    }

    boost::beast::error_code ec;
    auto const results =
        resolver.resolve(uri_result->host(), uri_result->port().empty() ? k_default_port : uri_result->port(), ec);
    if (ec) {
        return ec;
    }

    stream.connect(results, ec);
    if (ec) {
        return ec;
    }

    auto request = http::request<http::string_body>(method, uri_result->path(), 11);
    request.set(http::field::host, uri_result->host());
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request.set(http::field::accept, "*/*");
    if (!body.empty()) {
        request.set(http::field::content_type, "application/json");
        request.body() = body;
        request.prepare_payload();
    }

    http::write(stream, request, ec);
    if (ec) {
        return ec;
    }

    boost::beast::flat_buffer buffer;
    http::response<http::string_body> response;
    http::read(stream, buffer, response, ec);
    if (ec) {
        return ec;
    }

    stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

    if (ec && ec != boost::beast::errc::not_connected) {
        return ec;
    }

    return response;
}

void rav::HttpClient::get_async(boost::asio::io_context& io_context, const std::string& url, CallbackType callback) {
    request_async(io_context, http::verb::get, url, std::move(callback));
}

void rav::HttpClient::request_async(
    boost::asio::io_context& io_context, const http::verb method, const std::string& url,
    std::function<void(boost::system::result<http::response<http::string_body>> response)> callback
) {
    const auto uri = boost::urls::parse_uri(url);

    if (uri.has_error()) {
        callback(uri.error());
        return;
    }

    auto request = http::request<http::empty_body>(method, uri->path(), 11);
    request.set(http::field::host, uri->host());
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request.set(http::field::accept, "*/*");

    const auto session = std::make_shared<Session>(io_context);
    session->request(std::move(request), uri->host(), uri->port(), std::move(callback));
}

rav::HttpClient::Session::Session(boost::asio::io_context& io_context) : resolver_(io_context), stream_(io_context) {}

void rav::HttpClient::Session::request(
    http::request<http::empty_body> request, const std::string_view host, const std::string_view port,
    CallbackType callback
) {
    request_ = std::move(request);
    callback_ = std::move(callback);
    resolver_.async_resolve(
        host, port.empty() ? k_default_port : port,
        boost::beast::bind_front_handler(&Session::on_resolve, shared_from_this())
    );
}

void rav::HttpClient::Session::on_resolve(
    const boost::beast::error_code& ec, const boost::asio::ip::tcp::resolver::results_type& results
) {
    if (ec) {
        callback_(ec);
    }

    // Set a timeout on the operation
    stream_.expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    stream_.async_connect(results, boost::beast::bind_front_handler(&Session::on_connect, shared_from_this()));
}

void rav::HttpClient::Session::
    on_connect(const boost::beast::error_code& ec, const boost::asio::ip::tcp::resolver::results_type::endpoint_type&) {
    if (ec) {
        return callback_(ec);
    }

    // Set a timeout on the operation
    stream_.expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    http::async_write(stream_, request_, boost::beast::bind_front_handler(&Session::on_write, shared_from_this()));
}

void rav::HttpClient::Session::on_write(const boost::beast::error_code& ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        return callback_(ec);
    }

    // Receive the HTTP response
    http::async_read(
        stream_, buffer_, response_, boost::beast::bind_front_handler(&Session::on_read, shared_from_this())
    );
}

void rav::HttpClient::Session::on_read(boost::beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) {
        return callback_(ec);
    }

    // Gracefully close the socket
    stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes so don't bother reporting it.
    if (ec && ec != boost::beast::errc::not_connected) {
        return callback_(ec);
    }

    callback_(response_);
}