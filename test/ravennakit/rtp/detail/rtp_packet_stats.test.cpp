/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#include "ravennakit/rtp/rtp_packet.hpp"
#include "ravennakit/rtp/detail/rtp_packet_stats.hpp"

#include <catch2/catch_all.hpp>

TEST_CASE("rtp_packet_stats") {
    rav::rtp_packet_stats stats;

    SECTION("Basic sequence") {
        stats.update(10);
        stats.update(11);
        stats.update(12);
        stats.update(13);
        REQUIRE(stats.size() == 4);
        const auto counts = stats.collect();
        REQUIRE(counts.dropped == 0);
        REQUIRE(counts.duplicates == 0);
        REQUIRE(counts.out_of_order == 0);
        REQUIRE(counts.too_old == 0);
    }

    SECTION("Drop one packet") {
        stats.update(10);
        stats.update(12);
        REQUIRE(stats.size() == 3);
        const auto counts = stats.collect();
        REQUIRE(counts.dropped == 1);
        REQUIRE(counts.duplicates == 0);
        REQUIRE(counts.out_of_order == 0);
        REQUIRE(counts.too_old == 0);
    }

    SECTION("Drop two packets") {
        stats.update(10);
        stats.update(13);
        REQUIRE(stats.size() == 4);
        const auto counts = stats.collect();
        REQUIRE(counts.dropped == 2);
        REQUIRE(counts.duplicates == 0);
        REQUIRE(counts.out_of_order == 0);
        REQUIRE(counts.too_old == 0);
    }

    SECTION("A packet older than the first packet is dropped") {
        stats.update(10);
        stats.update(9);
        REQUIRE(stats.size() == 1);
        const auto counts = stats.collect();
        REQUIRE(counts.dropped == 0);
        REQUIRE(counts.duplicates == 0);
        REQUIRE(counts.out_of_order == 0);
        REQUIRE(counts.too_old == 0);
    }

    SECTION("Too old") {
        stats.update(10);
        stats.update(12);
        stats.update(11);
        REQUIRE(stats.size() == 3);
        const auto counts = stats.collect();
        REQUIRE(counts.dropped == 0);
        REQUIRE(counts.duplicates == 0);
        REQUIRE(counts.out_of_order == 0);
        REQUIRE(counts.too_old == 1);
    }

    SECTION("Too old after setting max age") {
        stats.set_max_age(2);
        stats.update(10);
        stats.update(14);
        stats.update(11); // Too old
        stats.update(13); // Out of order
        stats.update(13); // Our of order and duplicate
        REQUIRE(stats.size() == 5);
        const auto counts = stats.collect();
        REQUIRE(counts.dropped == 1); // Seq 12 is dropped
        REQUIRE(counts.duplicates == 1);
        REQUIRE(counts.out_of_order == 2);
        REQUIRE(counts.too_old == 1);
    }

    SECTION("Test the window") {
        stats.reset(4);
        stats.set_max_age(2);
        stats.update(10);
        stats.update(14);
        stats.update(11); // Too old
        stats.update(13); // Out of order
        stats.update(13); // Our of order and duplicate
        REQUIRE(stats.size() == 4);
        auto counts = stats.collect();
        REQUIRE(counts.dropped == 1); // Seq 12 is dropped
        REQUIRE(counts.duplicates == 1);
        REQUIRE(counts.out_of_order == 2);
        REQUIRE(counts.too_old == 1);

        // The window will slide one position, dropping seq 11 (which was marked too old).
        stats.update(15);
        REQUIRE(stats.size() == 4);
        counts = stats.collect();
        REQUIRE(counts.dropped == 1); // Seq 12 is dropped
        REQUIRE(counts.duplicates == 1);
        REQUIRE(counts.out_of_order == 2);
        REQUIRE(counts.too_old == 0);

        // The window will slide three positions, dropping previous packets except 15.
        stats.update(16);
        stats.update(17);
        stats.update(18);
        REQUIRE(stats.size() == 4);
        counts = stats.collect();
        REQUIRE(counts.dropped == 0);
        REQUIRE(counts.duplicates == 0);
        REQUIRE(counts.out_of_order == 0);
        REQUIRE(counts.too_old == 0);
    }

    SECTION("Test wrap around") {
        stats.reset(4);
        stats.set_max_age(2);
        stats.update(0xffff - 2);
        stats.update(0xffff - 1);
        stats.update(0xffff);
        stats.update(0x0);
        REQUIRE(stats.size() == 4);
        auto counts = stats.collect();
        REQUIRE(counts.dropped == 0);
        REQUIRE(counts.duplicates == 0);
        REQUIRE(counts.out_of_order == 0);
        REQUIRE(counts.too_old == 0);
    }

    SECTION("Test wrap around with drop") {
        stats.reset(4);
        stats.set_max_age(2);
        stats.update(0xffff - 2);
        stats.update(0xffff - 1);
        stats.update(0xffff);
        stats.update(0x1);
        REQUIRE(stats.size() == 4);
        auto counts = stats.collect();
        REQUIRE(counts.dropped == 1);
        REQUIRE(counts.duplicates == 0);
        REQUIRE(counts.out_of_order == 0);
        REQUIRE(counts.too_old == 0);
    }

    SECTION("Test wrap around with drop, out of order, duplicates and too old") {
        stats.reset(4);
        stats.set_max_age(2);
        stats.update(0xffff - 2);
        stats.update(0x1);
        stats.update(0x1);
        stats.update(0xffff - 1);
        stats.update(0xffff);
        auto counts = stats.collect();
        REQUIRE(counts.dropped == 1);
        REQUIRE(counts.duplicates == 1);
        REQUIRE(counts.out_of_order == 1);
        REQUIRE(counts.too_old == 1);
    }
}
