/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/rtsp/rtsp_client.hpp"

#include "ravennakit/core/log.hpp"
#include "ravennakit/rtsp/rtsp_request.hpp"

rav::rtsp_client::rtsp_client(asio::io_context& io_context) : socket_(asio::make_strand(io_context)) {
    input_data_.resize(1024);
}

void rav::rtsp_client::connect(const asio::ip::tcp::endpoint& endpoint) {
    asio::post(socket_.get_executor(), [this, endpoint]() {
        async_connect(endpoint);
    });
}

void rav::rtsp_client::async_connect(const asio::ip::tcp::endpoint& endpoint) {
    socket_.async_connect(endpoint, [this](const asio::error_code ec) {
        if (ec) {
            RAV_ERROR("Connect error: {}", ec.message());
            return;
        }
        RAV_INFO("Connected to {}", socket_.remote_endpoint().address().to_string());
        async_write_request();
        async_read_some();
    });
}

void rav::rtsp_client::async_write_request() {
    rtsp_request request;
    request.method = "DESCRIBE";
    request.uri = "rtsp://192.168.15.52/by-id/13";
    request.rtsp_version_major = 1;
    request.rtsp_version_minor = 0;
    request.headers["CSeq"] = "15";
    request.headers["Accept"] = "application/sdp";
    auto data = request.encode();

    asio::async_write(socket_, asio::buffer(data), [this](const asio::error_code ec, std::size_t length) {
        if (ec) {
            RAV_ERROR("Write error: {}", ec.message());
            return;
        }
        RAV_INFO("Wrote {} bytes", length);
    });
}

void rav::rtsp_client::async_read_some() {
    socket_.async_read_some(asio::buffer(input_data_), [this](const asio::error_code ec, std::size_t length) mutable {
        if (ec) {
            RAV_ERROR("Read error: {}", ec.message());
            // TODO: Close the connection?
            return;
        }

        auto input_begin = input_data_.begin();
        const auto input_end = input_data_.begin() + static_cast<long>(length);

        while (true) {
            auto [result, begin] = response_parser_.parse(input_begin, input_end);

            if (result == rav::rtsp_response_parser::result::good) {
                // TODO: We got a full request. Call some subscriber or something.
                RAV_INFO("Received response:\n{}", rav::string_replace(response_.encode(), "\r\n", "\n"));
                response_parser_.reset();
            } else if (result != rtsp_response_parser::result::indeterminate) {
                // TODO: Send back 400 Bad Request and terminate the connection
                RAV_ERROR("Error: invalid header ({})", static_cast<int>(result));
                RAV_ERROR("Received:\n{}", rav::string_replace(std::string(input_begin, input_end), "\r\n", "\n"));
                return;
            }

            if (begin < input_end) {
                input_begin = begin;
                continue;  // There is still data available to read, do another round.
            }

            break;  // Done reading
        }

        async_read_some();
    });
}
