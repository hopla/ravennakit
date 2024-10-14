/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include <catch2/catch_all.hpp>

#include "ravennakit/rtsp/rtsp_request.hpp"

TEST_CASE("rtsp_request", "[rtsp_request]") {
    SECTION("reset") {
        rav::rtsp_request request;
        request.method = "GET";
        request.uri = "/index.html";
        request.rtsp_version_major = 1;
        request.rtsp_version_minor = 1;
        request.headers.emplace_back({"CSeq", "1"});
        request.data = "Hello, World!";
        request.reset();
        REQUIRE(request.method.empty());
        REQUIRE(request.uri.empty());
        REQUIRE(request.rtsp_version_major == 0);
        REQUIRE(request.rtsp_version_minor == 0);
        REQUIRE(request.headers.empty());
        REQUIRE(request.data.empty());
    }
}
