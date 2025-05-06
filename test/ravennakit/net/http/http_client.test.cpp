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
    SECTION("GET sync") {
        boost::asio::io_context io_context;
        auto response = rav::HttpClient::get(io_context, "http://httpbin.cpp.al/get");
        io_context.run();

        REQUIRE(response.has_value());
        REQUIRE(response->result() == boost::beast::http::status::ok);
        REQUIRE(response->body().size() > 0);

        fmt::print("Response: {}\n", *response);

        auto json_body = nlohmann::json::parse(response->body());
        REQUIRE(json_body.at("url") == "http://httpbin.cpp.al/get");
    }

    SECTION("GET async") {
        boost::asio::io_context io_context;
        int counter = 0;
        rav::HttpClient::get_async(
            io_context, "http://httpbin.cpp.al/get",
            [&counter](boost::system::result<boost::beast::http::response<boost::beast::http::string_body>> response) {
                counter++;
                REQUIRE(response.has_value());
                REQUIRE(response->result() == boost::beast::http::status::ok);
                REQUIRE_FALSE(response->body().empty());
                fmt::print("Response: {}\n", *response);

                auto json_body = nlohmann::json::parse(response->body());
                REQUIRE(json_body.at("url") == "http://httpbin.cpp.al/get");
            }
        );
        io_context.run();
        REQUIRE(counter == 1);
    }
}
