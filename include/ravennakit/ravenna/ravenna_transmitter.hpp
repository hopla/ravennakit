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

#include "ravennakit/dnssd/dnssd_advertiser.hpp"
#include "ravennakit/rtsp/rtsp_server.hpp"

namespace rav {

class ravenna_transmitter {
  public:
    ravenna_transmitter(
        dnssd::dnssd_advertiser& advertiser, rtsp_server& rtsp_server, const util::id id, std::string session_name
    ) :
        advertiser_(advertiser), rtsp_server_(rtsp_server), id_(id), session_name_(std::move(session_name)) {
        advertisement_id_ = advertiser_.register_service(
            "_rtsp._tcp,_ravenna_session", session_name_.c_str(), nullptr, rtsp_server.port(), {}, false, false
        );
    }

    ~ravenna_transmitter() {
        if (advertisement_id_.is_valid()) {
            advertiser_.unregister_service(advertisement_id_);
        }
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
    util::id advertisement_id_;
};

}  // namespace rav
