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

#include "ravennakit/core/chrono/high_resolution_clock.hpp"

#include <cstdint>

namespace rav {

class ptp_local_clock {
public:
    [[nodiscard]] static uint64_t now() {
        return high_resolution_clock::now();
    }
};

}
