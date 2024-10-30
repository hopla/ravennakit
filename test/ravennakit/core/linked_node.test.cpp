/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/core/linked_node.hpp"

#include <iostream>
#include <catch2/catch_all.hpp>
#include <fmt/base.h>

TEST_CASE("linked_node | Build a list", "[linked_node]") {
    rav::linked_node n1(1);
    rav::linked_node n2(2);
    rav::linked_node n3(3);

    std::vector<int> nodes;

    SECTION("Single node") {
        REQUIRE(n1.value() == 1);
        REQUIRE(n1.is_front() == false);
        REQUIRE(n1.is_back() == false);
        REQUIRE(n1.is_linked() == false);

        REQUIRE(n2.value() == 2);
        REQUIRE(n2.is_front() == false);
        REQUIRE(n2.is_back() == false);
        REQUIRE(n2.is_linked() == false);

        REQUIRE(n3.value() == 3);
        REQUIRE(n3.is_front() == false);
        REQUIRE(n3.is_back() == false);
        REQUIRE(n3.is_linked() == false);

        for (const auto& node : n1) {
            nodes.push_back(node.value());
        }

        REQUIRE(nodes == std::vector {1});
    }

    n1.push_back(n2);

    SECTION("Two nodes") {
        REQUIRE(n1.value() == 1);
        REQUIRE(n1.is_front() == true);
        REQUIRE(n1.is_back() == false);
        REQUIRE(n1.is_linked() == true);

        REQUIRE(n2.value() == 2);
        REQUIRE(n2.is_front() == false);
        REQUIRE(n2.is_back() == true);
        REQUIRE(n2.is_linked() == true);

        REQUIRE(n3.value() == 3);
        REQUIRE(n3.is_front() == false);
        REQUIRE(n3.is_back() == false);
        REQUIRE(n3.is_linked() == false);

        for (const auto& node : n1) {
            nodes.push_back(node.value());
        }

        REQUIRE(nodes == std::vector {1, 2});
    }

    n1.push_back(n3);

    SECTION("Three nodes") {
        REQUIRE(n1.value() == 1);
        REQUIRE(n1.is_front() == true);
        REQUIRE(n1.is_back() == false);
        REQUIRE(n1.is_linked() == true);

        REQUIRE(n2.value() == 2);
        REQUIRE(n2.is_front() == false);
        REQUIRE(n2.is_back() == false);
        REQUIRE(n2.is_linked() == true);

        REQUIRE(n3.value() == 3);
        REQUIRE(n3.is_front() == false);
        REQUIRE(n3.is_back() == true);
        REQUIRE(n3.is_linked() == true);

        for (const auto& node : n1) {
            nodes.push_back(node.value());
        }

        REQUIRE(nodes == std::vector {1, 2, 3});
    }

    n2.remove();

    SECTION("Two nodes again") {
        REQUIRE(n1.value() == 1);
        REQUIRE(n1.is_front() == true);
        REQUIRE(n1.is_back() == false);
        REQUIRE(n1.is_linked() == true);

        REQUIRE(n2.value() == 2);
        REQUIRE(n2.is_front() == false);
        REQUIRE(n2.is_back() == false);
        REQUIRE(n2.is_linked() == false);

        REQUIRE(n3.value() == 3);
        REQUIRE(n3.is_front() == false);
        REQUIRE(n3.is_back() == true);
        REQUIRE(n3.is_linked() == true);

        for (const auto& node : n1) {
            nodes.push_back(node.value());
        }

        REQUIRE(nodes == std::vector {1, 3});
    }

    n1.remove();

    SECTION("One node again") {
        REQUIRE(n1.value() == 1);
        REQUIRE(n1.is_front() == false);
        REQUIRE(n1.is_back() == false);
        REQUIRE(n1.is_linked() == false);

        REQUIRE(n2.value() == 2);
        REQUIRE(n2.is_front() == false);
        REQUIRE(n2.is_back() == false);
        REQUIRE(n2.is_linked() == false);

        REQUIRE(n3.value() == 3);
        REQUIRE(n3.is_front() == false);
        REQUIRE(n3.is_back() == false);
        REQUIRE(n3.is_linked() == false);

        for (const auto& node : n1) {
            nodes.push_back(node.value());
        }

        REQUIRE(nodes == std::vector {1});

        nodes.clear();

        for (const auto& node : n2) {
            nodes.push_back(node.value());
        }

        REQUIRE(nodes == std::vector {2});

        nodes.clear();

        for (const auto& node : n3) {
            nodes.push_back(node.value());
        }

        REQUIRE(nodes == std::vector {3});
    }
}

TEST_CASE("linked_node | try to break it", "[linked_node]") {
    rav::linked_node n1(1);
    rav::linked_node n2(2);
    rav::linked_node n3(3);

    std::vector<int> nodes;

    n1.push_back(n2);
    n1.push_back(n3);

    SECTION("Adding a node twice should keep integrity") {
        n1.push_back(n2);

        for (const auto& node : n1) {
            nodes.push_back(node.value());
        }

        REQUIRE(nodes == std::vector {1, 3, 2});
    }

    SECTION("When a node goes out of scope it should remove itself") {
        {
            rav::linked_node n4(4);
            n1.push_back(n4);

            for (const auto& node : n1) {
                nodes.push_back(node.value());
            }
            REQUIRE(nodes == std::vector {1, 2, 3, 4});
        }

        nodes.clear();
        for (const auto& node : n1) {
            nodes.push_back(node.value());
        }
        REQUIRE(nodes == std::vector {1, 2, 3});
    }
}

TEST_CASE("linked_node | Assign new value", "[linked_node]") {
    rav::linked_node n1(1);
    n1 = 4;
    REQUIRE(n1.value() == 4);
}

TEST_CASE("linked_node | move semantics") {
    rav::linked_node n1("n1");
    rav::linked_node n2("n2");
    rav::linked_node n3("n3");
    n1.push_back(n2);
    n1.push_back(n3);

    std::vector<std::string> nodes;

    SECTION("Move assignment") {
        rav::linked_node l1("l1");
        rav::linked_node l2("l2");
        rav::linked_node l3("l3");
        l1.push_back(l2);
        l1.push_back(l3);
        l2 = std::move(n2);
        for (const auto& node : n1) {
            if (node.value() != nullptr) {
                nodes.emplace_back(node.value());
            } else {
                nodes.emplace_back("nullptr");
            }
        }
        for (const auto& node : l1) {
            if (node.value() != nullptr) {
                nodes.emplace_back(node.value());
            } else {
                nodes.emplace_back("nullptr");
            }
        }
        REQUIRE(nodes == std::vector<std::string> {"n1", "nullptr", "n3", "l1", "n2", "l3"});
    }

    SECTION("Move construction") {
        rav::linked_node l1("l1");
        rav::linked_node l2(std::move(n2));
        rav::linked_node l3("l3");
        l1.push_back(l2);
        l1.push_back(l3);
        for (const auto& node : n1) {
            if (node.value() != nullptr) {
                nodes.emplace_back(node.value());
            } else {
                nodes.emplace_back("nullptr");
            }
        }
        for (const auto& node : l1) {
            if (node.value() != nullptr) {
                nodes.emplace_back(node.value());
            } else {
                nodes.emplace_back("nullptr");
            }
        }
        REQUIRE(nodes == std::vector<std::string> {"n1", "nullptr", "n3", "l1", "n2", "l3"});
    }

    SECTION("Swap") {
        rav::linked_node l1("l1");
        rav::linked_node l2("l2");
        rav::linked_node l3("l3");
        l1.push_back(l2);
        l1.push_back(l3);
        std::swap(n2, l2);
        for (const auto& node : n1) {
            nodes.emplace_back(node.value());
        }
        for (const auto& node : l1) {
            nodes.emplace_back(node.value());
        }
        REQUIRE(nodes == std::vector<std::string> {"n1", "l2", "n3", "l1", "n2", "l3"});
    }
}
