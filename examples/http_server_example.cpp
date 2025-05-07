/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#include "ravennakit/core/log.hpp"
#include "ravennakit/core/net/http/http_server.hpp"

int main() {
    boost::asio::io_context io_context;

    // Create a server instance
    rav::HttpServer server(io_context);

    // Start the server
    const auto result = server.start("127.0.0.1", 8080);
    if (result.has_error()) {
        RAV_ERROR("Error starting server: {}", result.error().message());
        return 1;
    }

    // Run the io_context to start accepting connections
    io_context.run();

    return 0;
}
