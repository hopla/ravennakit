/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/sdp/media_clock.hpp"

#include "ravennakit/core/log.hpp"
#include "ravennakit/core/string_parser.hpp"

rav::sdp::media_clock::parse_result<rav::sdp::media_clock> rav::sdp::media_clock::parse_new(const std::string_view line
) {
    string_parser parser(line);

    media_clock clock;

    if (const auto mode_part = parser.read_until(' ')) {
        string_parser mode_parser(*mode_part);

        if (const auto mode = mode_parser.read_until('=')) {
            if (mode == "direct") {
                clock.mode_ = clock_mode::direct;
            } else {
                RAV_WARNING("Unsupported media clock mode: {}", *mode);
                return parse_result<media_clock>::err("media_clock: unsupported media clock mode");
            }
        } else {
            return parse_result<media_clock>::err("media_clock: invalid media clock mode");
        }

        if (!mode_parser.exhausted()) {
            if (const auto offset = mode_parser.read_int<int64_t>()) {
                clock.offset_ = *offset;
            } else {
                return parse_result<media_clock>::err("media_clock: invalid offset");
            }
        }
    }

    if (parser.exhausted()) {
        return parse_result<media_clock>::ok(clock);
    }

    if (const auto rate = parser.read_until('=')) {
        if (rate == "rate") {
            const auto numerator = parser.read_int<int32_t>();
            if (!numerator) {
                return parse_result<media_clock>::err("media_clock: invalid rate numerator");
            }
            if (!parser.skip('/')) {
                return parse_result<media_clock>::err("media_clock: invalid rate denominator");
            }
            const auto denominator = parser.read_int<int32_t>();
            if (!denominator) {
                return parse_result<media_clock>::err("media_clock: invalid rate denominator");
            }
            clock.rate_ = fraction<int32_t> {*numerator, *denominator};
        } else {
            return parse_result<media_clock>::err("media_clock: unexpected token");
        }
    } else {
        return parse_result<media_clock>::err("media_clock: expecting rate");
    }

    return parse_result<media_clock>::ok(clock);
}

rav::sdp::media_clock::clock_mode rav::sdp::media_clock::mode() const {
    return mode_;
}

std::optional<int64_t> rav::sdp::media_clock::offset() const {
    return offset_;
}

const std::optional<rav::fraction<int>>& rav::sdp::media_clock::rate() const {
    return rate_;
}
