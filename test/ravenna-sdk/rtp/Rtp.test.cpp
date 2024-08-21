/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravenna-sdk/rtp/Rtp.hpp"

#include <catch2/catch_all.hpp>

TEST_CASE("Rtp | rtcp_packet_type_to_string()") {
    auto expect = [](const rav::rtp::RtcpPacketType packet_type, const char* str) {
        return std::strcmp(rtcp_packet_type_to_string(packet_type), str) == 0;
    };

    REQUIRE(expect(rav::rtp::RtcpPacketType::SourceDescriptionItems, "SourceDescriptionItems"));
    REQUIRE(expect(rav::rtp::RtcpPacketType::SenderReport, "SenderReport"));
    REQUIRE(expect(rav::rtp::RtcpPacketType::ReceiverReport, "ReceiverReport"));
    REQUIRE(expect(rav::rtp::RtcpPacketType::Unknown, "Unknown"));
    REQUIRE(expect(rav::rtp::RtcpPacketType::Bye, "Bye"));
    REQUIRE(expect(rav::rtp::RtcpPacketType::App, "App"));
}
