/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#include "ravennakit/core/net/sockets/extended_udp_socket.hpp"
#include "ravennakit/core/util/id.hpp"
#include "ravennakit/nmos/nmos_node.hpp"
#include "ravennakit/rtp/rtp_audio_receiver.hpp"

#include <boost/asio.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/container/static_vector.hpp>

int main() {
    const auto multicast_addr = boost::asio::ip::make_address("239.15.1.5");
    const auto interface_addr = boost::asio::ip::make_address_v4("192.168.15.54");

    boost::asio::io_context io_context;
    const boost::asio::ip::udp::endpoint endpoint(interface_addr, 5004);

    boost::asio::ip::udp::socket socket1(io_context);
    socket1.open(boost::asio::ip::udp::v4());
    socket1.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket1.bind(endpoint);
    socket1.non_blocking(true);
    socket1.set_option(boost::asio::ip::multicast::join_group(multicast_addr.to_v4(), interface_addr));
    socket1.set_option(boost::asio::detail::socket_option::integer<IPPROTO_IP, IP_RECVDSTADDR_PKTINFO>(1));

    boost::asio::ip::udp::socket socket2(io_context);
    socket2.open(boost::asio::ip::udp::v4());
    socket2.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket2.bind(endpoint);
    socket2.non_blocking(true);
    socket2.set_option(boost::asio::ip::multicast::join_group(multicast_addr.to_v4(), interface_addr));
    socket2.set_option(boost::asio::detail::socket_option::integer<IPPROTO_IP, IP_RECVDSTADDR_PKTINFO>(1));

    std::array<uint8_t, 1500> buffer {};
    int num_packets_socket1 = 0;
    int num_packets_socket2 = 0;

    boost::asio::ip::udp::endpoint src_endpoint;
    boost::asio::ip::udp::endpoint dst_endpoint;
    uint64_t recv_time = 0;
    boost::system::error_code ec;

    while (true) {
        if (socket1.is_open() && socket1.available()) {
            auto len = rav::receive_from_socket(socket1, buffer, src_endpoint, dst_endpoint, recv_time, ec);
            if (ec) {
                throw boost::system::system_error(ec);
            }
            if (len > 0) {
                num_packets_socket1++;
                if (!dst_endpoint.address().is_multicast()) {
                    fmt::println("Non multicast packet on socket 1");
                }
            }
        }

        if (socket2.is_open() && socket2.available()) {
            auto len = rav::receive_from_socket(socket2, buffer, src_endpoint, dst_endpoint, recv_time, ec);
            if (ec) {
                throw boost::system::system_error(ec);
            }
            if (len > 0) {
                num_packets_socket2++;
                if (!dst_endpoint.address().is_multicast()) {
                    fmt::println("Non multicast packet on socket 2");
                }
            }
        }

        if (num_packets_socket1 > 3000) {
            socket1.close();
        }

        if (num_packets_socket2 > 6000) {
            break;
        }
    }

    fmt::println("Number of packets: {} | {}", num_packets_socket1, num_packets_socket2);

    return 0;
}
