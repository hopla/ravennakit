/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#include "ravennakit/core/system.hpp"
#include "ravennakit/rtp/detail/udp_sender_receiver.hpp"

int main() {
    // Is it possible to receive 2 multicast groups on the same socket?
    // Is it possible to receive 2 multicast groups on the same socket on 2 different interfaces?

    rav::set_log_level_from_env();
    rav::do_system_checks();

    asio::io_context io_context;

    const auto interface1 = asio::ip::make_address_v4("192.168.15.53");
    const auto interface2 = asio::ip::make_address_v4("192.168.15.56");

    const rav::rtp::UdpSenderReceiver socket(io_context, asio::ip::address_v4::any(), 5004);

    socket.start([](const rav::rtp::UdpSenderReceiver::recv_event& event) {
        RAV_TRACE("{}:{}", event.dst_endpoint.address().to_string(), event.dst_endpoint.port());
    });

    auto subscription1 = socket.join_multicast_group(asio::ip::make_address_v4("239.15.55.1"), interface1);
    auto subscription2 = socket.join_multicast_group(asio::ip::make_address_v4("239.15.55.2"), interface2);

    io_context.run();

    return 0;
}
