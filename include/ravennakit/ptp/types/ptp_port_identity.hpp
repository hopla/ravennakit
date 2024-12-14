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

#include "ptp_clock_identity.hpp"
#include "ravennakit/core/log.hpp"

namespace rav {

/**
 * Represents a PTP port identity.
 * IEEE1588-2019: 5.3.5
 */
struct ptp_port_identity {
    constexpr static uint16_t k_port_number_min = 0x1;     // Inclusive
    constexpr static uint16_t k_port_number_max = 0xfffe;  // Inclusive
    constexpr static uint16_t k_port_number_all = 0xffff;  // Means all ports

    ptp_clock_identity clock_identity;
    uint16_t port_number {};  // Valid range: [k_port_number_min, k_port_number_max]

    /**
     * Checks the internal state of the identity according to IEEE1588-2019. Asserts when something is wrong.
     */
    void assert_valid_state() const {
        clock_identity.assert_valid_state();
        RAV_ASSERT(port_number >= k_port_number_min, "port_number is below minimum");
        RAV_ASSERT(port_number <= k_port_number_max, "port_number is above maximum");
    }

    friend bool operator==(const ptp_port_identity& lhs, const ptp_port_identity& rhs) {
        return std::tie(lhs.clock_identity, lhs.port_number) == std::tie(rhs.clock_identity, rhs.port_number);
    }

    friend bool operator!=(const ptp_port_identity& lhs, const ptp_port_identity& rhs) {
        return !(lhs == rhs);
    }
};

}  // namespace rav
