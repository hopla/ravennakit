/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#include "ravennakit/core/sync/realtime_shared_list.hpp"

#include <catch2/catch_all.hpp>

TEST_CASE("realtime_shared_list") {
    SECTION("Basic operation") {
        rav::realtime_shared_list<std::string> list;
        REQUIRE(list.push_back("Hello world"));

        auto lock = list.lock_realtime();
        REQUIRE(lock);
        REQUIRE(lock.size() == 1);
        REQUIRE(lock[0] == "Hello world");
        REQUIRE(*lock.at(0) == "Hello world");

        // This operation fails because the lock is still active.
        REQUIRE_FALSE(list.erase(0));

        lock.reset();
        REQUIRE_FALSE(lock);
        REQUIRE(lock.size() == 0);
        REQUIRE(lock.at(0) == nullptr);

        // Now that the lock is no longer active, the operation succeeds.
        REQUIRE(list.erase(0));

        auto lock2 = list.lock_realtime();
        REQUIRE(lock2.empty());
    }

    SECTION("Clear") {
        rav::realtime_shared_list<std::string> list;
        REQUIRE(list.push_back("1"));
        REQUIRE(list.push_back("2"));

        {
            auto lock = list.lock_realtime();
            REQUIRE(lock.size() == 2);

            // The lock is active so clearing fails
            REQUIRE_FALSE(list.clear());
            REQUIRE(lock.size() == 2);
            REQUIRE(lock[0] == "1");
            REQUIRE(lock[1] == "2");
        }

        REQUIRE(list.clear());

        auto lock = list.lock_realtime();
        REQUIRE(lock.empty());
    }

    SECTION("Range based for") {
        rav::realtime_shared_list<std::string> list;
        REQUIRE(list.push_back("1"));
        REQUIRE(list.push_back("2"));

        auto lock = list.lock_realtime();

        std::vector<std::string> values;
        for (const auto* v : lock) {
            values.push_back(*v);
        }

        REQUIRE(values.size() == 2);
        REQUIRE(values[0] == "1");
        REQUIRE(values[1] == "2");

        lock.reset();
        values.clear();

        // Even when the lock has been reset (and doesn't contain a value), the range based for should not crash.
        for ([[maybe_unused]] const auto* v : lock) {
            FAIL("Should not be reached");
        }
    }
}
