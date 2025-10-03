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
#include "ptp_comparison_data_set.hpp"
#include "ravennakit/ptp/messages/ptp_announce_message.hpp"

namespace rav::ptp {

struct BestAnnounceMessage {
    AnnounceMessage message;
    PortIdentity receiver_identity;

    [[nodiscard]] ComparisonDataSet get_comparison_data_set() const {
        return {message, receiver_identity};
    }
};

}  // namespace rav::ptp
