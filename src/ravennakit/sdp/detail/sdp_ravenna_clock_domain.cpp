/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/sdp/detail/sdp_ravenna_clock_domain.hpp"

#include "ravennakit/core/string_parser.hpp"

rav::sdp::ravenna_clock_domain::parse_result<rav::sdp::ravenna_clock_domain>
rav::sdp::ravenna_clock_domain::parse_new(const std::string_view line) {
    string_parser parser(line);

    ravenna_clock_domain clock_domain;

    if (const auto sync_source = parser.split(' ')) {
        if (sync_source == "PTPv2") {
            if (const auto domain = parser.read_int<int32_t>()) {
                clock_domain = ravenna_clock_domain {sync_source::ptp_v2, *domain};
            } else {
                return parse_result<ravenna_clock_domain>::err("clock_domain: invalid domain");
            }
        } else {
            return parse_result<ravenna_clock_domain>::err("clock_domain: unsupported sync source");
        }
    } else {
        return parse_result<ravenna_clock_domain>::err("clock_domain: failed to parse sync source");
    }

    return parse_result<ravenna_clock_domain>::ok(clock_domain);
}
