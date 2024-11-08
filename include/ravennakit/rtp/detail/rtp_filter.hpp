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

#include "ravennakit/sdp/constants.hpp"

#include <asio.hpp>

namespace rav {

/**
 * Implements logic for filtering RTP packets.
 */
class rtp_filter {
  public:
    [[nodiscard]] bool matches(const asio::ip::udp::endpoint& dst_endpoint) const {
        return connection_address_ == dst_endpoint;
    }

    [[nodiscard]] bool
    matches(const asio::ip::udp::endpoint& dst_endpoint, const asio::ip::address& src_address) const {
        if (connection_address_ != dst_endpoint) {
            return false;
        }

        if (filters_.empty()) {
            return true;
        }

        bool is_address_included = false;
        bool has_include_filters = false;

        for (auto& filter : filters_) {
            if (filter.mode == sdp::filter_mode::exclude && filter.address == src_address) {
                return false;  // This implementation prioritizes exclude filters over include filters
            }
            if (filter.mode == sdp::filter_mode::include) {
                has_include_filters = true;
                if (filter.address == src_address) {
                    is_address_included = true;
                }
            }
        }

        return has_include_filters ? is_address_included : true;
    }

  private:
    struct filter {
        sdp::filter_mode mode {sdp::filter_mode::undefined};
        asio::ip::address address;
    };

    asio::ip::udp::endpoint connection_address_;
    std::vector<filter> filters_;
};

}  // namespace rav
