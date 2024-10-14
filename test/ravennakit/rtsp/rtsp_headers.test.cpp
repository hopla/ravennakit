/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/rtsp/rtsp_request.hpp"

#include <catch2/catch_all.hpp>

TEST_CASE("rtsp_headers", "[rtsp_headers]") {
    SECTION("Get header value") {
        rav::rtsp_headers headers;
        REQUIRE(headers.get_header_value("CSeq") == nullptr);
        headers.push_back({"CSeq", "1"});
        auto* value = headers.get_header_value("CSeq");
        REQUIRE(value != nullptr);
        REQUIRE(*value == "1");
    }

    SECTION("Get content length") {
        rav::rtsp_headers headers;
        REQUIRE(headers.get_content_length() == std::nullopt);
        headers.emplace_back({"Content-Length", "10"});
        REQUIRE(headers.get_content_length() == 10);
    }

    SECTION("reset") {
        rav::rtsp_headers headers;
        headers.push_back({"CSeq", "1"});
        headers.push_back({"Content-Length", "10"});
        headers.clear();
        REQUIRE(headers.empty());
    }
}
