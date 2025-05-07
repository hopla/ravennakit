/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#include "ravennakit/core/json.hpp"
#include "ravennakit/core/net/http/http_client.hpp"

#include <iostream>
#include <catch2/catch_all.hpp>
#include <fmt/format.h>

TEST_CASE("HttpClient") {
    SECTION("Shorthand get") {
        boost::asio::io_context io_context;
        const auto response = rav::HttpClient(io_context, "http://httpbin.cpp.al/get").get();
        io_context.run();
        REQUIRE(response.has_value());
        REQUIRE(response->result() == boost::beast::http::status::ok);
        REQUIRE(response->body().size() > 0);

        auto json_body = nlohmann::json::parse(response->body());
        REQUIRE(json_body.at("url") == "http://httpbin.cpp.al/get");
    }

    SECTION("Shorthand get without path") {
        boost::asio::io_context io_context;
        const auto response = rav::HttpClient(io_context, "http://httpbin.cpp.al").get();
        io_context.run();
        REQUIRE(response.has_value());
        REQUIRE(response->result() == boost::beast::http::status::ok);
    }

    SECTION("Shorthand get but with different path") {
        boost::asio::io_context io_context;
        const auto response = rav::HttpClient(io_context, "http://httpbin.cpp.al/get").get("/anything");
        io_context.run();
        REQUIRE(response.has_value());
        REQUIRE(response->result() == boost::beast::http::status::ok);

        auto body = response->body();
        auto json_body = nlohmann::json::parse(response->body());
        REQUIRE(json_body.at("url") == "http://httpbin.cpp.al/anything");
    }

    SECTION("Shorthand get_async") {
        boost::asio::io_context io_context;

        int counter = 0;
        auto token =
            [&counter](
                const boost::system::result<boost::beast::http::response<boost::beast::http::string_body>>& response
            ) {
                REQUIRE(response.has_value());
                REQUIRE(response->result() == boost::beast::http::status::ok);
                REQUIRE(response->body().size() > 0);

                auto json_body = nlohmann::json::parse(response->body());
                REQUIRE(json_body.at("url") == "http://httpbin.cpp.al/get");
                counter++;
            };

        rav::HttpClient(io_context, "http://httpbin.cpp.al/get").get_async(token);
        io_context.run();

        REQUIRE(counter == 1);
    }
}
