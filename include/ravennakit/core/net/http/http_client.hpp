/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#pragma once

#include "ravennakit/core/net/http/detail/http_fmt_adapters.hpp"

#include <boost/asio.hpp>
#include <boost/url.hpp>

#include <string>

namespace rav {

namespace http = boost::beast::http;
using tcp = boost::asio::ip::tcp;

/**
 * This wrapper class providing a simple interface for making HTTP requests.
 */
class HttpClient {
  public:
    /// When no port is specified in the urls, the default port is used.
    static constexpr auto k_default_port = "80";

    using CallbackType = std::function<void(boost::system::result<http::response<http::string_body>> response)>;

    /**
     * Synchronous GET request.
     * @param io_context The io_context to use for the request.
     * @param url The URL to request.
     * @return The response from the server, which may contain an error.
     */
    static boost::system::result<http::response<http::string_body>>
    get(boost::asio::io_context& io_context, const std::string& url);

    /**
     * Synchronous GET request.
     * @param io_context The io_context to use for the request.
     * @param url The URL to request.
     * @param body The body of the request.
     * @return The response from the server, which may contain an error.
     */
    static boost::system::result<http::response<http::string_body>>
    post(boost::asio::io_context& io_context, const std::string& url, const std::string& body);

    /**
     * Make a synchronous HTTP request.
     * @param io_context The io_context to use for the request.
     * @param method The HTTP method to use.
     * @param url The host to connect to.
     * @param body The body of the request.
     * @return The response from the server, which may contain an error.
     */
    static boost::system::result<http::response<http::string_body>>
    request(boost::asio::io_context& io_context, http::verb method, std::string_view url, std::string_view body);

    /**
     * Asynchronous GET request.
     *
     * The lifetime of the callback will be tied to the given io_context and will outlive the call to this function.
     *
     * @param io_context The io_context to use for the request.
     * @param url The URL to request.
     * @param callback The callback to call when the request is complete which is when the response is received or when
     * an error occurs.
     */
    static void get_async(boost::asio::io_context& io_context, const std::string& url, CallbackType callback);

    /**
     * Asynchronous GET request.
     *
     * The lifetime of the callback will be tied to the given io_context and will outlive the call to this function.
     *
     * @param io_context The io_context to use for the request.
     * @param method
     * @param url The URL to request.
     * @param callback The callback to call when the request is complete which is when the response is received or when
     * an error occurs.
     */
    static void request_async(
        boost::asio::io_context& io_context, http::verb method, const std::string& url, CallbackType callback
    );

  private:
    /**
     * A session class that keeps itself alive and handles the connection and request/response cycle.
     */
    class Session: public std::enable_shared_from_this<Session> {
      public:
        explicit Session(boost::asio::io_context& io_context);

        void request(
            http::request<http::empty_body> request, std::string_view host, std::string_view port, CallbackType callback
        );

      private:
        http::request<http::empty_body> request_;
        http::response<http::string_body> response_;
        boost::asio::ip::tcp::resolver resolver_;
        boost::beast::tcp_stream stream_;
        boost::beast::flat_buffer buffer_;
        CallbackType callback_;

        void on_resolve(const boost::beast::error_code& ec, const tcp::resolver::results_type& results);
        void on_connect(const boost::beast::error_code& ec, const tcp::resolver::results_type::endpoint_type&);
        void on_write(const boost::beast::error_code& ec, std::size_t bytes_transferred);
        void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);
    };
};

}  // namespace rav
