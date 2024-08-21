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

#include <cstddef>
#include <cstdint>
#include <string>

#include "Rtp.hpp"
#include "ravenna-sdk/ntp/TimeStamp.hpp"

namespace rav {

class RtcpPacketView {
  public:
    /**
     * Constructs an RTP header from the given data.
     * @param data The RTP header data.
     * @param data_length The lenght of the RTP header data in bytes.
     */
    RtcpPacketView(const uint8_t* data, size_t data_length);

    /**
     * Verifies the RTP header data. After this method returns all other methods should return valid data and not lead
     * to undefined behavior.
     * @returns The result of the verification.
     */
    [[nodiscard]] rtp::VerificationResult verify() const;

    /**
     * @returns The version of the RTP header.
     */
    [[nodiscard]] uint8_t version() const;

    /**
     * @returns True if the padding bit is set.
     */
    [[nodiscard]] bool padding() const;

    /**
     * @returns The reception report count. Should always be higher than zero.
     */
    [[nodiscard]] uint8_t reception_report_count() const;

    /**
     * @return The packet type
     */
    [[nodiscard]] rtp::RtcpPacketType packet_type() const;

    /**
     * @returns The length of this RTCP packet in 32-bit words.
     */
    [[nodiscard]] uint16_t length() const;

    /**
     * @return The synchronization source identifier.
     */
    [[nodiscard]] uint32_t ssrc() const;

    /**
     * @return The NTP timestamp of a SR or RR packet. If the packet is not an SR or RR then the value returned will be
     * zero.
     */
    [[nodiscard]] ntp::TimeStamp ntp_timestamp() const;

    /**
     * @returns A string representation of the RTCP header.
     */
    [[nodiscard]] std::string to_string() const;

  private:
    const uint8_t* data_ {};
    size_t data_length_ {0};
};

}  // namespace rav
