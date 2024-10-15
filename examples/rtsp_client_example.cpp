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
#include "ravennakit/core/log.hpp"
#include "ravennakit/core/system.hpp"
#include "ravennakit/rtsp/rtsp_client.hpp"

#include <CLI/App.hpp>

int main(int const argc, char* argv[]) {
#if RAV_ENABLE_SPDLOG
    spdlog::set_level(spdlog::level::trace);
#endif

    rav::system::do_system_checks();

    CLI::App app {"RTSP Client example"};
    argv = app.ensure_utf8(argv);

    std::string addr;
    app.add_option("address", addr, "The address to connect to")->required();

    CLI11_PARSE(app, argc, argv);

    asio::io_context io_context;

    rav::rtsp_client client(io_context);
    client.connect({asio::ip::make_address(addr), 80});

    io_context.run();
}
