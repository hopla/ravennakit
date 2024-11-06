/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/core/log.hpp"
#include "ravennakit/rtp/rtcp_packet_view.hpp"
#include "ravennakit/rtp/rtp_receiver.hpp"

#include "ravennakit/core/subscriber_list.hpp"
#include "ravennakit/rtp/rtp_packet_view.hpp"
#include "ravennakit/core/tracy.hpp"

#include <fmt/core.h>

#include <utility>

rav::rtp_receiver::rtp_receiver(asio::io_context& io_context) : io_context_(io_context) {}

rav::rtp_receiver::~rtp_receiver() = default;

void rav::rtp_receiver::subscriber::subscribe_to_rtp_session(
    rtp_receiver& receiver, const asio::ip::address& address, const uint16_t port
) {
    *node_ = {this, &receiver};

    const asio::ip::udp::endpoint endpoint(address, port);

    for (const auto& session : receiver.sessions_) {
        if (session->connection_endpoint() == endpoint) {
            session->subscribe(node_);
            return;
        }
    }

    // Create new session
    const auto& it = receiver.sessions_.emplace_back(std::make_unique<session>(receiver.io_context_, endpoint));
    it->subscribe(node_);
    it->start();
}

rav::rtp_receiver::session::session(asio::io_context& io_context, asio::ip::udp::endpoint endpoint) :
    io_context_(io_context), connection_endpoint_(std::move(endpoint)) {}

rav::rtp_receiver::session::~session() {
    stop();
    for (auto& node : subscriber_nodes_) {
        node.reset();
    }
}

void rav::rtp_receiver::session::start(const asio::ip::address& interface_addr) {
    if (rtp_endpoint_) {
        RAV_WARNING("RTP endpoint already started");
        return;
    }
    rtp_endpoint_ = rtp_endpoint::make_rtp_endpoint(io_context_, interface_addr, connection_endpoint_.port());
    rtp_endpoint_->start(*this);

    RAV_TRACE(
        "Session started for connection {}:{}", connection_endpoint_.address().to_string(), connection_endpoint_.port()
    );
}

void rav::rtp_receiver::session::stop() {
    rtp_endpoint_->stop();
    rtp_endpoint_.reset();

    RAV_TRACE(
        "Session stopped for endpoint {}:{}", connection_endpoint_.address().to_string(), connection_endpoint_.port()
    );
}

void rav::rtp_receiver::session::on(const rtp_packet_event& rtp_event) {
    RAV_TRACE("{}", rtp_event.packet.to_string());
}

void rav::rtp_receiver::session::on(const rtcp_packet_event& rtcp_event) {
    RAV_TRACE("{}", rtcp_event.packet.to_string());
}

asio::ip::udp::endpoint rav::rtp_receiver::session::connection_endpoint() const {
    if (!rtp_endpoint_) {
        return {};
    }
    return connection_endpoint_;
}

void rav::rtp_receiver::session::subscribe(subscriber::node_type& node) {
    subscriber_nodes_.push_back(node);
}
