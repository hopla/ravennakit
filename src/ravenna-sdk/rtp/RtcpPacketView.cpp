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

#include <fmt/core.h>

#include "ravenna-sdk/platform/ByteOrder.hpp"

namespace {
constexpr size_t kRtcpHeaderBaseLengthOctets = 8;
constexpr size_t kRtcpNtpTimestampLength = 8;
}  // namespace

rav::RtcpPacketView::RtcpPacketView(const uint8_t* data, const size_t data_length) :
    data_(data), data_length_(data_length) {}

rav::rtp::VerificationResult rav::RtcpPacketView::verify() const {
    if (data_ == nullptr) {
        return rtp::VerificationResult::InvalidPointer;
    }

    if (data_length_ < kRtcpHeaderBaseLengthOctets) {
        return rtp::VerificationResult::InvalidHeaderLength;
    }

    if (version() > 2) {
        return rtp::VerificationResult::InvalidVersion;
    }

    return rtp::VerificationResult::Ok;
}

uint8_t rav::RtcpPacketView::version() const {
    if (data_length_ < 1) {
        return 0;
    }
    return (data_[0] & 0b11000000) >> 6;
}

bool rav::RtcpPacketView::padding() const {
    if (data_length_ < 1) {
        return false;
    }
    return (data_[0] & 0b00100000) >> 5 != 0;
}

uint8_t rav::RtcpPacketView::reception_report_count() const {
    if (data_length_ < 1) {
        return false;
    }
    return data_[0] & 0b00011111;
}

rav::rtp::RtcpPacketType rav::RtcpPacketView::packet_type() const {
    if (data_length_ < 2) {
        return rtp::RtcpPacketType::Unknown;
    }

    switch (data_[1]) {
        case 200:
            return rtp::RtcpPacketType::SenderReport;
        case 201:
            return rtp::RtcpPacketType::ReceiverReport;
        case 202:
            return rtp::RtcpPacketType::SourceDescriptionItems;
        case 203:
            return rtp::RtcpPacketType::Bye;
        case 204:
            return rtp::RtcpPacketType::App;
        default:
            return rtp::RtcpPacketType::Unknown;
    }
}

uint16_t rav::RtcpPacketView::length() const {
    constexpr auto kOffset = 2;
    if (data_length_ < kOffset + sizeof(uint16_t)) {
        return 0;
    }

    return byte_order::read_be<uint16_t>(&data_[kOffset]) + 1;
}

uint32_t rav::RtcpPacketView::ssrc() const {
    constexpr auto kOffset = 4;
    if (data_length_ < kOffset + sizeof(uint32_t)) {
        return 0;
    }
    return byte_order::read_be<uint32_t>(&data_[kOffset]);
}

rav::ntp::TimeStamp rav::RtcpPacketView::ntp_timestamp() const {
    if (packet_type() != rtp::RtcpPacketType::SenderReport) {
        return {};
    }

    if (data_length_ < kRtcpHeaderBaseLengthOctets + kRtcpNtpTimestampLength) {
        return {};
    }

    return {
        byte_order::read_be<uint32_t>(data_ + kRtcpHeaderBaseLengthOctets),
        byte_order::read_be<uint32_t>(data_ + kRtcpHeaderBaseLengthOctets + sizeof(uint32_t))
    };
}

std::string rav::RtcpPacketView::to_string() const {
    auto header = fmt::format(
        "RTCP Packet: valid={} version={} padding={} reception_report_count={} packet_type={} length={} ssrc={}",
        verify() == rtp::VerificationResult::Ok,
        version(),
        padding(),
        reception_report_count(),
        rtcp_packet_type_to_string(packet_type()),
        length(),
        ssrc()
    );

    if (packet_type() == rtp::RtcpPacketType::SenderReport) {}

    return header;
}
