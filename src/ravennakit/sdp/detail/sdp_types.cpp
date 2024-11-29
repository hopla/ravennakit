/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/sdp/detail/sdp_types.hpp"

std::string rav::sdp::to_string(const filter_mode& filter_mode) {
    switch (filter_mode) {
        case filter_mode::exclude:
            return "excl";
        case filter_mode::include:
            return "incl";
        case filter_mode::undefined:
            return "undefined";
        default:
            return "undef";
    }
}
