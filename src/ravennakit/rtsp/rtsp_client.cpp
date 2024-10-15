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

rav::rtsp_client::rtsp_client(asio::io_context& io_context) : socket_(asio::make_strand(io_context)) {}

void rav::rtsp_client::connect(const asio::ip::tcp::endpoint& endpoint) {
    asio::post(socket_.get_executor(), [this, endpoint]() {
        async_connect(endpoint);
    });
}

void rav::rtsp_client::async_connect(const asio::ip::tcp::endpoint& endpoint) {
    socket_.async_connect(endpoint, [](const asio::error_code ec) {
        if (ec) {
            RAV_ERROR("Connect error: {}", ec.message());
            return;
        }
        RAV_INFO("Connected");
    });
}
