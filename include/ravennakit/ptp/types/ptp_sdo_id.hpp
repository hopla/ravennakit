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

namespace rav::ptp {

struct SdoId {
    uint8_t major {};
    uint8_t minor {};

    [[nodiscard]] std::string to_string() const {
        return fmt::format("{}.{}", major, minor);
    }

    friend bool operator==(const SdoId& lhs, const SdoId& rhs) {
        return std::tie(lhs.major, lhs.minor) == std::tie(rhs.major, rhs.minor);
    }

    friend bool operator!=(const SdoId& lhs, const SdoId& rhs) {
        return !(lhs == rhs);
    }
};

}
