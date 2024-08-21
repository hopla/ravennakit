/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravenna-sdk/rtp/RtcpPacketView.hpp"

#include <asio/detail/socket_ops.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("RtcpPacketView | verify()", "[RtcpPacketView]") {
    uint8_t data[] = {

        // v, p, rc
        0b10'0'10101,
        // packet type
        200,
        0,
        0,
        0,
        0,
        0,
        0
    };

    SECTION("Invalid pointer") {
        const rav::RtcpPacketView packet(nullptr, sizeof(data));
        REQUIRE(packet.verify() == rav::rtp::VerificationResult::InvalidPointer);
    }

    SECTION("Invalid header length") {
        const rav::RtcpPacketView packet(data, 0);
        REQUIRE(packet.verify() == rav::rtp::VerificationResult::InvalidHeaderLength);
    }

    const rav::RtcpPacketView packet(data, sizeof(data));

    SECTION("Ok") {
        REQUIRE(packet.verify() == rav::rtp::VerificationResult::Ok);
    }

    SECTION("Valid version 0") {
        data[0] = 0;
        REQUIRE(packet.version() == 0);
        REQUIRE(packet.verify() == rav::rtp::VerificationResult::Ok);
    }

    SECTION("Valid version 1") {
        data[0] = 0b01000000;
        REQUIRE(packet.version() == 1);
        REQUIRE(packet.verify() == rav::rtp::VerificationResult::Ok);
    }

    SECTION("Valid version 2") {
        data[0] = 0b10000000;
        REQUIRE(packet.version() == 2);
        REQUIRE(packet.verify() == rav::rtp::VerificationResult::Ok);
    }

    SECTION("Invalid version 3") {
        data[0] = 0b11000000;
        REQUIRE(packet.version() == 3);
        REQUIRE(packet.verify() == rav::rtp::VerificationResult::InvalidVersion);
    }
}

TEST_CASE("RtcpPacketView | version()", "[RtcpPacketView]") {
    uint8_t data[] = {0b00'0'10101};

    const rav::RtcpPacketView packet(data, sizeof(data));

    SECTION("Version 0") {
        REQUIRE(packet.version() == 0);
    }

    SECTION("Version 1") {
        data[0] = 0b01111111;
        REQUIRE(packet.version() == 1);
    }

    SECTION("Version 2") {
        data[0] = 0b10111111;
        REQUIRE(packet.version() == 2);
    }

    SECTION("Version 3") {
        data[0] = 0b11111111;
        REQUIRE(packet.version() == 3);
    }
}

TEST_CASE("RtcpPacketView | padding()", "[RtcpPacketView]") {
    uint8_t data[] = {0b11'0'11111};

    const rav::RtcpPacketView packet(data, sizeof(data));

    SECTION("No padding") {
        REQUIRE(packet.padding() == false);
    }

    SECTION("With padding") {
        data[0] = 0b11111111;
        REQUIRE(packet.padding() == true);
    }
}

TEST_CASE("RtcpPacketView | reception_report_count()", "[RtcpPacketView]") {
    uint8_t data[] = {0b11'1'00000};
    const rav::RtcpPacketView packet(data, sizeof(data));

    SECTION("Count 0") {
        REQUIRE(packet.reception_report_count() == 0);
    }

    SECTION("Count 15") {
        data[0] = {0b11'1'10101};
        REQUIRE(packet.reception_report_count() == 0x15);
    }

    SECTION("Count 31 (max value)") {
        data[0] = {0b11'1'11111};
        REQUIRE(packet.reception_report_count() == 0x1f);
    }
}

TEST_CASE("RtcpPacketView | packet_type()", "[RtcpPacketView]") {
    uint8_t data[] = {0b11111111, 0};
    const rav::RtcpPacketView packet(data, sizeof(data));

    REQUIRE(packet.packet_type() == rav::rtp::RtcpPacketType::Unknown);

    SECTION("Sender report") {
        data[1] = 200;
        REQUIRE(packet.packet_type() == rav::rtp::RtcpPacketType::SenderReport);
    }

    SECTION("Receiver report") {
        data[1] = 201;
        REQUIRE(packet.packet_type() == rav::rtp::RtcpPacketType::ReceiverReport);
    }

    SECTION("Source description items") {
        data[1] = 202;
        REQUIRE(packet.packet_type() == rav::rtp::RtcpPacketType::SourceDescriptionItems);
    }

    SECTION("Bye") {
        data[1] = 203;
        REQUIRE(packet.packet_type() == rav::rtp::RtcpPacketType::Bye);
    }

    SECTION("App specific") {
        data[1] = 204;
        REQUIRE(packet.packet_type() == rav::rtp::RtcpPacketType::App);
    }
}

TEST_CASE("RtcpPacketView | length()", "[RtcpPacketView]") {
    uint8_t data[] = {0xFF, 0xFF, 0xAB, 0xCD};
    const rav::RtcpPacketView packet(data, sizeof(data));

    SECTION("0xABCD") {
        REQUIRE(packet.length() == 0xABCE);
    }

    SECTION("0x0") {
        data[2] = 0;
        data[3] = 0;
        REQUIRE(packet.length() == 0x1);  // Length is encoded minus one
    }

    SECTION("0xFFFF") {
        data[2] = 0xFF;
        data[3] = 0xFE;
        REQUIRE(packet.length() == 0xFFFF);
    }
}

TEST_CASE("RtcpPacketView | ssrc()", "[RtcpPacketView]") {
    uint8_t data[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x02, 0x03, 0x04};
    const rav::RtcpPacketView packet(data, sizeof(data));

    SECTION("0x01020304") {
        REQUIRE(packet.ssrc() == 0x01020304);
    }

    SECTION("0x0") {
        data[4] = 0;
        data[5] = 0;
        data[6] = 0;
        data[7] = 0;
        REQUIRE(packet.ssrc() == 0x0);
    }

    SECTION("0xFFFF") {
        data[4] = 0xFF;
        data[5] = 0xFF;
        data[6] = 0xFF;
        data[7] = 0xFF;
        REQUIRE(packet.ssrc() == 0xFFFFFFFF);
    }
}

TEST_CASE("RtcpPacketView | to_string()", "[RtcpPacketView]") {
    uint8_t data[] = {// v, p, rc
                      0b10'0'10101,
                      // packet type
                      200,
                      // length
                      0xAB,
                      0XCD,
                      // csrc
                      0x01,
                      0x02,
                      0x03,
                      0x04
    };

    const rav::RtcpPacketView packet(data, sizeof(data));

    SECTION("Test to_string method") {
        REQUIRE(
            packet.to_string()
            == "RTCP Packet: valid=true version=2 padding=false reception_report_count=21 packet_type=SenderReport length=43982 ssrc=16909060"
        );
    }
}

TEST_CASE("RtcpPacketView | ntp_timestamp()", "[RtcpPacketView]") {
    uint8_t data[] = {

        // v, p, rc
        0b10'0'10101,
        // packet type
        200,
        // length
        0xAB,
        0XCD,
        // csrc
        0x01,
        0x02,
        0x03,
        0x04,
        // NTP MSW
        0x01,
        0x02,
        0x03,
        0x04,
        /// NTP LSW
        0x05,
        0x06,
        0x07,
        0x08,
    };

    const rav::RtcpPacketView packet(data, sizeof(data));

    SECTION("Sender report") {
        const auto [integer, fraction] = packet.ntp_timestamp();
        REQUIRE(integer == 16909060);
        REQUIRE(fraction == 84281096);
    }

    SECTION("Receiver report") {
        data[1] = 201;
        const auto [integer, fraction] = packet.ntp_timestamp();
        REQUIRE(integer == 0);
        REQUIRE(fraction == 0);
    }
}
