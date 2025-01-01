/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#include "ravennakit/ptp/types/ptp_timestamp.hpp"

#include <catch2/catch_all.hpp>

TEST_CASE("ptp_timestamp") {
    SECTION("Construct from nanos") {
        rav::ptp_timestamp ts(1'000'000'000);
        REQUIRE(ts.seconds == 1);
        REQUIRE(ts.nanoseconds == 0);
    }

    SECTION("Construct from nanos 2") {
        rav::ptp_timestamp ts(1'000'000'001);
        REQUIRE(ts.seconds == 1);
        REQUIRE(ts.nanoseconds == 1);
    }

    SECTION("Construct from max nanos value") {
        rav::ptp_timestamp ts(std::numeric_limits<uint64_t>::max());
        REQUIRE(ts.seconds == 18446744073);
        REQUIRE(ts.nanoseconds == 709551615);
    }
}
