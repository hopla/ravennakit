/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#pragma once
#include <cstdint>

namespace rav {

class rtp_stream {
  public:
    rtp_stream() = delete;

    explicit rtp_stream(const uint32_t ssrc) : ssrc_(ssrc) {}

    rtp_stream(const rtp_stream&) = delete;
    rtp_stream& operator=(const rtp_stream&) = delete;

    rtp_stream(rtp_stream&&) noexcept = default;
    rtp_stream& operator=(rtp_stream&&) noexcept = default;

    [[nodiscard]] uint32_t ssrc() const {
        return ssrc_;
    }

  private:
    uint32_t ssrc_ {};
};

}  // namespace rav
