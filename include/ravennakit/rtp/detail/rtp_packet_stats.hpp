/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#pragma once
#include "ravennakit/core/containers/ring_buffer.hpp"
#include "ravennakit/core/containers/detail/fifo.hpp"
#include "ravennakit/core/util/wrapping_uint.hpp"
#include "ravennakit/rtp/rtp_packet_view.hpp"

namespace rav {

/**
 * A class that collects statistics about RTP packets.
 */
class rtp_packet_stats {
  public:
    struct counters {
        uint16_t out_of_order {};
        uint16_t too_old {};
        uint16_t duplicates {};
        uint16_t dropped {};
    };

    explicit rtp_packet_stats() = default;

    /**
     * @param window_size The window size in number of packets. Max value is 65535 (0xffff).
     */
    explicit rtp_packet_stats(const size_t window_size) : window_(window_size) {
        RAV_ASSERT(window_size <= std::numeric_limits<uint16_t>::max(), "Window size too large");
    }

    /**
     * Updates the statistics with the given packet.
     * @param sequence_number
     * @return True if packet should be processed, or false if it should be dropped because it is too old.
     */
    void update(const uint16_t sequence_number) {
        const auto packet_sequence_number = wrapping_uint16(sequence_number);

        if (!most_recent_sequence_number_.has_value()) {
            most_recent_sequence_number_ = packet_sequence_number - 1;
        }

        const auto diff = most_recent_sequence_number_->update(sequence_number);
        for (uint16_t i = 0; i < diff; ++i) {
            window_.push_back({});
        }

        const auto offset = (*most_recent_sequence_number_ - sequence_number).value();

        if (offset >= window_.size()) {
            return;  // Too old for the window
        }

        auto& packet = window_[window_.size() - 1 - offset];
        packet.times_received++;
        if (packet_sequence_number < *most_recent_sequence_number_ - max_age_) {
            packet.times_too_old++;  // Packet older than max_age_
        } else if (packet_sequence_number < *most_recent_sequence_number_) {
            packet.times_out_of_order++;  // Packet out of order
        }
    }

    /**
     * Collects the statistics, counting the number of dropped, out of order, too old, and duplicate packets for the
     * current window.
     * @return The collected statistics.
     */
    counters collect() {
        if (window_.empty()) {
            return {};
        }

        counters result {};
        for (const auto& packet : window_) {
            if (packet.times_received == 0) {
                result.dropped++;
            } else if (packet.times_received > 1) {
                result.duplicates = packet.times_received - 1;
            }
            result.out_of_order += packet.times_out_of_order;
            result.too_old += packet.times_too_old;
        }
        return result;
    }

    /**
     * Sets the maximum age of a packet. When the packet is older than max age it will be marked as expired.
     * @param max_age The maximum age in number of packets.
     */
    void set_max_age(const uint16_t max_age) {
        max_age_ = max_age;
    }

    /**
     * @return The number of packets in the window.
     */
    [[nodiscard]] size_t size() const {
        return window_.size();
    }

    /**
     * Resets to the initial state.
     * @param window_size The window size in number of packets. Max value is 65535 (0xffff).
     */
    void reset(const std::optional<uint16_t> window_size = {}) {
        if (window_size.has_value()) {
            RAV_ASSERT(*window_size <= std::numeric_limits<uint16_t>::max(), "Window size too large");
        }
        window_.reset(window_size);
        most_recent_sequence_number_ = {};
    }

  private:
    struct packet {
        uint16_t times_received {};
        uint16_t times_out_of_order {};
        uint16_t times_too_old {};
    };

    std::optional<wrapping_uint16> most_recent_sequence_number_ {};
    uint16_t max_age_ {};
    ring_buffer<packet> window_ {32};
};

}  // namespace rav
