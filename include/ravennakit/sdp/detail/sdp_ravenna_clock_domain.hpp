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

#include "ravennakit/core/result.hpp"

#include <cstdint>
#include <string_view>

namespace rav::sdp {

/**
 * Defines a clock source and domain. This is a RAVENNA-specific attribute extension to the SDP specification.
 */
struct ravenna_clock_domain {
    static constexpr auto k_attribute_name = "clock-domain";
    enum class sync_source { undefined, ptp_v2 };

    /// A type alias for a parse result.
    template<class T>
    using parse_result = result<T, std::string>;

    sync_source source {sync_source::undefined};
    int32_t domain {};

    static parse_result<ravenna_clock_domain> parse_new(std::string_view line);
};

}
