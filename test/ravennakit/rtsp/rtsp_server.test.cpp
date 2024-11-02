/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/asio/io_context_runner.hpp"
#include "ravennakit/rtsp/rtsp_client.hpp"
#include "ravennakit/rtsp/rtsp_server.hpp"

#include <catch2/catch_all.hpp>
#include <thread>

TEST_CASE("rtsp_server", "[rtsp_server]") {
    SECTION("Port") {
        asio::io_context io_context;

        std::thread thread([&] {
            io_context.run();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        SECTION("Any port") {
            rav::rtsp_server server(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), 0));
            REQUIRE(server.port() != 0);
            server.close();
        }

        SECTION("Specific port") {
            rav::rtsp_server server(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), 555));
            REQUIRE(server.port() == 555);
            server.close();
        }

        thread.join();
    }
}

TEST_CASE("rtsp_server | DESCRIBE", "[rtsp_server]") {
    asio::io_context io_context;
    int server_request_count = 0;
    int server_response_count = 0;
    rav::rtsp_server server(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), 0));
    server.on<rav::rtsp_connection::connection_event>([](const rav::rtsp_connection::connection_event& event) {
        rav::rtsp_request request;
        request.method = "ANNOUNCE";
        request.uri = "/";
        request.data = "announce request data";
        event.connection.async_send_request(request);
    });
    server.on<rav::rtsp_connection::request_event>([&](const rav::rtsp_connection::request_event& event) {
        RAV_TRACE("{}", event.request.to_debug_string(true));
        REQUIRE(event.request.method == "DESCRIBE");
        REQUIRE(event.request.uri == "rtsp://::1/");
        REQUIRE(event.request.data == "describe request data");
        event.connection.async_send_response(rav::rtsp_response(200, "OK", "describe response data"));
        server_request_count++;
    });
    server.on<rav::rtsp_connection::response_event>([&](const rav::rtsp_connection::response_event& event) {
        RAV_TRACE("{}", event.response.to_debug_string(true));
        REQUIRE(event.response.status_code == 200);
        REQUIRE(event.response.reason_phrase == "OK");
        REQUIRE(event.response.data == "announce response data");
        server_response_count++;
        server.close();
        // io_context.stop();
    });

    const auto port = server.port();
    REQUIRE(port != 0);

    int client_request_count = 0;
    int client_response_count = 0;
    rav::rtsp_client client(io_context);
    client.on<rav::rtsp_connection::connection_event>([](const rav::rtsp_connection::connection_event& event) {
        RAV_TRACE("Connected, send DESCRIBE request");
        event.connection.async_send_request(rav::rtsp_request("DESCRIBE", "rtsp://::1/", "describe request data"));
    });
    client.on<rav::rtsp_connection::request_event>(
        [&client_request_count](const rav::rtsp_connection::request_event& event) {
            RAV_INFO("{}", event.request.to_debug_string(true));
            REQUIRE(event.request.method == "ANNOUNCE");
            REQUIRE(event.request.uri == "/");
            REQUIRE(event.request.data == "announce request data");
            event.connection.async_send_response(rav::rtsp_response(200, "OK", "announce response data"));
            client_request_count++;
        }
    );
    std::string response_data;
    client.on<rav::rtsp_connection::response_event>(
        [&client_response_count](const rav::rtsp_connection::response_event& event) {
            RAV_INFO("{}", event.response.to_debug_string(true));
            REQUIRE(event.response.status_code == 200);
            REQUIRE(event.response.reason_phrase == "OK");
            REQUIRE(event.response.data == "describe response data");
            client_response_count++;
        }
    );
    client.async_connect("::1", port);

    io_context.run();

    REQUIRE(server_request_count == 1);
    REQUIRE(server_response_count == 1);
    REQUIRE(client_request_count == 1);
    REQUIRE(client_response_count == 1);
}
