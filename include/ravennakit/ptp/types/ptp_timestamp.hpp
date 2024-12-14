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

#include "ravennakit/core/types/uint48.hpp"

namespace rav {

struct ptp_timestamp {
    uint48_t seconds;
    uint32_t nanoseconds {};
};

using ptp_time_interval = int64_t;

}
