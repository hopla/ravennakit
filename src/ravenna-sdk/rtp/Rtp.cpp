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

const char* rav::rtp::rtcp_packet_type_to_string(const RtcpPacketType packet_type) {
    switch (packet_type) {
        case RtcpPacketType::SenderReport:
            return "SenderReport";
        case RtcpPacketType::ReceiverReport:
            return "ReceiverReport";
        case RtcpPacketType::SourceDescriptionItems:
            return "SourceDescriptionItems";
        case RtcpPacketType::Bye:
            return "Bye";
        case RtcpPacketType::App:
            return "App";
        case RtcpPacketType::Unknown:
            return "Unknown";
    }
    return "";
}
