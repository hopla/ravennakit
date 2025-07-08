/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#include "ravennakit/rtp/detail/rtp_receiver.hpp"

#include <catch2/catch_all.hpp>

TEST_CASE("rav::rtp::Receiver") {
    SECTION("Test bounds") {
        REQUIRE(rav::rtp::Receiver3::k_max_num_redundant_streams >= 1);
        REQUIRE(rav::rtp::Receiver3::k_max_num_rtp_sessions_per_stream >= 1);
        REQUIRE(
            rav::rtp::Receiver3::k_max_num_sessions
            == rav::rtp::Receiver3::k_max_num_redundant_streams * rav::rtp::Receiver3::k_max_num_rtp_sessions_per_stream
        );
    }

    SECTION("Initial state") {
        rav::rtp::Receiver3 receiver;

        // Sockets
        REQUIRE(receiver.sockets.capacity() == rav::rtp::Receiver3::k_max_num_sessions);
        REQUIRE(receiver.sockets.empty());

        // Streams
        REQUIRE(receiver.streams.size() == rav::rtp::Receiver3::k_max_num_redundant_streams);
    }

    SECTION("Add receiver") {
        rav::rtp::Receiver3::Sessions sessions {
            {{boost::asio::ip::make_address("239.0.0.1"), 5004, 5005},
             {boost::asio::ip::make_address("239.0.0.2"), 5004, 5005}}
        };

        rav::rtp::Receiver3::Filters filters {
            rav::rtp::Filter {boost::asio::ip::make_address("239.0.0.1")},
            rav::rtp::Filter {boost::asio::ip::make_address("239.0.0.2")},
        };

        rav::rtp::Receiver3 receiver;
        receiver.add_stream(rav::Id(1), sessions, filters);

        REQUIRE(receiver.streams.size() >= 2);  // For this test we need at least 2

        REQUIRE(receiver.streams[0].associated_id == rav::Id(1));
        REQUIRE(receiver.streams[0].sessions == sessions);
        REQUIRE(receiver.streams[0].filters == filters);
        REQUIRE(receiver.streams[0].status == rav::rtp::Receiver3::Status::added);

        REQUIRE_FALSE(receiver.streams[1].associated_id.is_valid());
        REQUIRE(receiver.streams[1].sessions == rav::rtp::Receiver3::Sessions {});
        REQUIRE(receiver.streams[1].filters == rav::rtp::Receiver3::Filters {});
        REQUIRE(receiver.streams[1].status == rav::rtp::Receiver3::Status::free);

        REQUIRE(receiver.sockets.empty());
    }

    SECTION("Binding a UDP socket to the any address") {
        boost::asio::io_context io_context;
        boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address_v4::any(), 0);
        boost::asio::ip::udp::socket socket(io_context, endpoint);
        REQUIRE(socket.is_open());
        REQUIRE(socket.local_endpoint().address() == endpoint.address());
        REQUIRE(socket.local_endpoint().port() != endpoint.port());
    }

    SECTION("Binding and joining a multicast address") {
        boost::asio::io_context io_context;
        auto multicast_addr = boost::asio::ip::make_address_v4("239.1.2.3");
        boost::asio::ip::udp::endpoint endpoint(multicast_addr, 0);
        boost::asio::ip::udp::socket socket(io_context, endpoint);
        REQUIRE(socket.is_open());
        REQUIRE(socket.local_endpoint().address() == endpoint.address());
        REQUIRE(socket.local_endpoint().port() != endpoint.port());
        socket.set_option(boost::asio::ip::multicast::join_group(multicast_addr));
    }

    SECTION("Send and receive UDP packets") {
        boost::asio::io_context io_context;
        boost::asio::ip::udp::socket rx(io_context, {boost::asio::ip::address_v4::loopback(), 0});
        REQUIRE(rx.is_open());

        std::array<char, 8> send_buffer {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x0A};

        boost::asio::ip::udp::socket tx(io_context, {boost::asio::ip::address_v4::loopback(), 0});
        tx.send_to(boost::asio::buffer(send_buffer), rx.local_endpoint());
        rx.send_to(boost::asio::buffer(send_buffer), rx.local_endpoint());

        std::array<char, 8> recv_buffer {};

        rx.receive(boost::asio::buffer(recv_buffer));
        REQUIRE(recv_buffer == send_buffer);

        rx.receive(boost::asio::buffer(recv_buffer));
        REQUIRE(recv_buffer == send_buffer);
    }

    SECTION("Add a multicast stream") {
        boost::asio::io_context io_context;
        rav::rtp::Receiver3 receiver;

        rav::rtp::Session session {boost::asio::ip::make_address("239.0.0.1"), 5004, 5005};
        rav::rtp::Filter filter {boost::asio::ip::make_address("239.0.0.1")};

        receiver.add_stream(rav::Id(1), {{session}}, {filter});
    }
}
