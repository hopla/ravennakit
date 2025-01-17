/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#pragma once

#include "ravennakit/core/uri.hpp"
#include "ravennakit/dnssd/dnssd_advertiser.hpp"
#include "ravennakit/rtsp/rtsp_server.hpp"
#include "ravennakit/sdp/sdp_session_description.hpp"

namespace rav {

class ravenna_transmitter {
  public:
    ravenna_transmitter(
        dnssd::dnssd_advertiser& advertiser, rtsp_server& rtsp_server, const util::id id, std::string session_name,
        asio::ip::address_v4 interface_address
    ) :
        advertiser_(advertiser),
        rtsp_server_(rtsp_server),
        id_(id),
        session_name_(std::move(session_name)),
        interface_address_(interface_address) {
        // Register handlers for the paths
        by_name_path_ = fmt::format("/by-name/{}", session_name_);
        by_id_path_ = fmt::format("/by-id/{}", id_.to_string());
        auto handler = [this](const rtsp_connection::request_event event) {
            on_request_event(event);
        };
        rtsp_server_.register_handler(by_name_path_, handler);
        rtsp_server_.register_handler(by_id_path_, handler);

        advertisement_id_ = advertiser_.register_service(
            "_rtsp._tcp,_ravenna_session", session_name_.c_str(), nullptr, rtsp_server.port(), {}, false, false
        );
    }

    ~ravenna_transmitter() {
        if (advertisement_id_.is_valid()) {
            advertiser_.unregister_service(advertisement_id_);
        }
        rtsp_server_.register_handler(by_name_path_, nullptr);
        rtsp_server_.register_handler(by_id_path_, nullptr);
    }

    ravenna_transmitter(const ravenna_transmitter& other) = delete;
    ravenna_transmitter& operator=(const ravenna_transmitter& other) = delete;

    ravenna_transmitter(ravenna_transmitter&& other) noexcept = delete;
    ravenna_transmitter& operator=(ravenna_transmitter&& other) noexcept = delete;

    [[nodiscard]] util::id id() const {
        return id_;
    }

    [[nodiscard]] std::string session_name() const {
        return session_name_;
    }

  private:
    dnssd::dnssd_advertiser& advertiser_;
    rtsp_server& rtsp_server_;
    util::id id_;
    std::string session_name_;
    asio::ip::address_v4 interface_address_;
    util::id advertisement_id_;
    std::string by_name_path_;
    std::string by_id_path_;
    int32_t clock_domain_ {};

    void on_request_event(const rtsp_connection::request_event event) {
        RAV_TRACE("Received request: {}", event.request.to_debug_string(false));
        const auto sdp = build_sdp();
        RAV_TRACE("SDP:\n{}", sdp.to_string("\n").value());
        const auto encoded = sdp.to_string();
        if (!encoded) {
            RAV_ERROR("Failed to encode SDP");
            return;
        }
        // event.connection.async_send_response(rtsp_response(200, "OK", *encoded));
    }

    sdp::session_description build_sdp() const {
        sdp::format format;
        format.payload_type = 98;


        sdp::media_description media;
        media.set_media_type("audio");
        media.set_port(5004);
        media.set_protocol("RTP/AVP");
        media.add_format(format);

        sdp::session_description sdp;

        // Origin
        const sdp::origin_field o {
            "-", id_.to_string(), 0, sdp::netw_type::internet, sdp::addr_type::ipv4, interface_address_.to_string()
        };
        sdp.set_origin(o);

        // Session name
        sdp.set_session_name(session_name_);

        // Connection info
        const sdp::connection_info_field c {
            sdp::netw_type::internet, sdp::addr_type::ipv4, interface_address_.to_string(), 15, {}
        };
        sdp.set_connection_info(c);

        // RAVENNA clock domain
        sdp.set_clock_domain(sdp::ravenna_clock_domain {sdp::ravenna_clock_domain::sync_source::ptp_v2, clock_domain_});

        // Reference clock
        // TODO: Fill in the GMID
        const sdp::reference_clock ref_clock {
            sdp::reference_clock::clock_source::ptp, sdp::reference_clock::ptp_ver::IEEE_1588_2008, "GMID",
            clock_domain_
        };
        sdp.set_ref_clock(ref_clock);

        // Media clock
        sdp::media_clock_source media_clk {sdp::media_clock_source::clock_mode::direct, 0, {}};
        sdp.set_media_clock(media_clk);

        // Add the stream to the session
        sdp.add_media_description(media);

        return sdp;
    }
};

}  // namespace rav
