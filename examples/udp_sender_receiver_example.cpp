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
#include "ravennakit/core/system.hpp"
#include "ravennakit/rtp/detail/udp_sender_receiver.hpp"

int main(int const argc, char* argv[]) {
    rav::log::set_level_from_env();
    rav::system::do_system_checks();

    const auto interface_addr = asio::ip::make_address("192.168.16.50");
    const auto multicast_addr = asio::ip::make_address("239.1.16.51");

    std::vector<rav::subscription> subscriptions;
    asio::io_context io_context;

    rav::udp_sender_receiver unicast1(io_context, asio::ip::address_v4(), 5004);
    unicast1.start([](const rav::udp_sender_receiver::recv_event& event) {
        RAV_TRACE(
            "Received on unicast1: {}:{} => {}:{}", event.src_endpoint.address().to_string(), event.src_endpoint.port(),
            event.dst_endpoint.address().to_string(), event.dst_endpoint.port()
        );
    });

    rav::udp_sender_receiver unicast2(io_context, interface_addr, 5004);
    unicast2.start([](const rav::udp_sender_receiver::recv_event& event) {
        RAV_TRACE(
            "Received on unicast2: {}:{} => {}:{}", event.src_endpoint.address().to_string(), event.src_endpoint.port(),
            event.dst_endpoint.address().to_string(), event.dst_endpoint.port()
        );
    });

    rav::udp_sender_receiver multicast1(io_context, multicast_addr, 5004);
    subscriptions.push_back(multicast1.join_multicast_group(multicast_addr, interface_addr));
    multicast1.start([](const rav::udp_sender_receiver::recv_event& event) {
        RAV_TRACE(
            "Received on rx2: {}:{} => {}:{}", event.src_endpoint.address().to_string(), event.src_endpoint.port(),
            event.dst_endpoint.address().to_string(), event.dst_endpoint.port()
        );
    });

    rav::udp_sender_receiver multicast2(io_context, multicast_addr, 5004);
    subscriptions.push_back(multicast2.join_multicast_group(multicast_addr, interface_addr));
    multicast2.start([](const rav::udp_sender_receiver::recv_event& event) {
        RAV_TRACE(
            "Received on rx3: {}:{} => {}:{}", event.src_endpoint.address().to_string(), event.src_endpoint.port(),
            event.dst_endpoint.address().to_string(), event.dst_endpoint.port()
        );
    });

    io_context.run();
}
