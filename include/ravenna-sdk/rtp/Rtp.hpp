/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#pragma once

namespace rav::rtp {

enum class VerificationResult {
    Ok,
    InvalidPointer,
    InvalidHeaderLength,
    InvalidVersion,
};

enum class RtcpPacketType {
    /// Unknown packet type
    Unknown,
    /// Sender report, for transmission and reception statistics from participants that are active senders
    SenderReport,
    /// Receiver report, for reception statistics from participants that are not active senders and in combination with
    /// SR for active senders reporting on more than 31 sources
    ReceiverReport,
    /// Source description items, including CNAME
    SourceDescriptionItems,
    /// Indicates end of participation
    Bye,
    /// Application-specific functions
    App
};

/**
 * @param packet_type The type to get a string representation for.
 * @return A string representation of given packet type.
 */
const char* rtcp_packet_type_to_string(RtcpPacketType packet_type);

}  // namespace rav::rtp
