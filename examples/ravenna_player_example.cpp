/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

/**
 * This examples demonstrates how to create a source and send audio onto the network. It does this by reading audio from
 * a wav file on disk and sending it as multicast audio packets.
 */

#include "ravennakit/core/file.hpp"
#include "ravennakit/core/log.hpp"
#include "ravennakit/core/system.hpp"
#include "ravennakit/dnssd/dnssd_advertiser.hpp"
#include "ravennakit/ravenna/ravenna_transmitter.hpp"

#include <CLI/App.hpp>
#include <asio/io_context.hpp>

class ravenna_player_example {
  public:
    explicit ravenna_player_example(asio::io_context& io_context, const uint16_t port_num) :
        rtsp_server_(io_context, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num)) {
        advertiser_ = rav::dnssd::dnssd_advertiser::create(io_context);
    }

    void add_source(const rav::file& file) {
        if (!file.exists()) {
            throw std::runtime_error("File does not exist: " + file.path().string());
        }
        const auto file_session_name = file.path().filename().string();
        for (const auto& source : sources_) {
            // Test if a source with the same session name already exists
            if (source.transmitter->session_name() == file_session_name) {
                throw std::runtime_error("A source with the same session name already exists: " + file_session_name);
            }
        }
        auto transmitter = std::make_unique<rav::ravenna_transmitter>(
            *advertiser_, rtsp_server_, id_generator_.next(), file_session_name
        );
        sources_.push_back({file, std::move(transmitter)});
    }

  private:
    struct source {
        rav::file file;
        std::unique_ptr<rav::ravenna_transmitter> transmitter;
    };

    std::unique_ptr<rav::dnssd::dnssd_advertiser> advertiser_;
    rav::rtsp_server rtsp_server_;
    rav::util::id::generator id_generator_ {};
    std::vector<source> sources_;
};

int main(int const argc, char* argv[]) {
    rav::log::set_level_from_env();
    rav::system::do_system_checks();

    CLI::App app {"RAVENNA Player example"};
    argv = app.ensure_utf8(argv);

    std::vector<std::string> file_paths;
    app.add_option("files", file_paths, "The files to stream")->required();

    std::string interface_address = "0.0.0.0";
    app.add_option("--interface-addr", interface_address, "The interface address");

    CLI11_PARSE(app, argc, argv);

    asio::io_context io_context;
    ravenna_player_example player_example(io_context, 5005);

    for (auto& file_path : file_paths) {
        player_example.add_source(rav::file(file_path));
    }

    io_context.run();
    return 0;
}
