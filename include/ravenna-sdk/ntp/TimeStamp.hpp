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

#include <cstdint>

namespace rav::ntp {

/**
 * Represents NTP wallclock time which is in seconds relative to 0h UTC on 1 January 1900. The full resolution NTP
 * timestamp is divided into an integer part (4 octets) and a fractional part (4 octets).
 */
struct TimeStamp {
    uint32_t integer;
    uint32_t fraction;
};

}
