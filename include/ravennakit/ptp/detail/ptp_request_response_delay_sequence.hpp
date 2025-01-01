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

#include "ravennakit/ptp/messages/ptp_delay_req_message.hpp"
#include "ravennakit/ptp/messages/ptp_follow_up_message.hpp"
#include "ravennakit/ptp/messages/ptp_sync_message.hpp"

namespace rav {

class ptp_request_response_delay_sequence {
public:
    ptp_request_response_delay_sequence() = default;

    ptp_request_response_delay_sequence(const ptp_sync_message& sync_message, const uint64_t t2) :
        sync_message_(sync_message), t2_(t2) {}

    [[nodiscard]] bool matches(const ptp_message_header& header) const {
        return sync_message_.header.matches(header);
    }

    void set_follow_up_message(const ptp_follow_up_message& follow_up_message) {
        follow_up_message_ = follow_up_message;
        t1_ = follow_up_message.precise_origin_timestamp;
    }

    [[nodiscard]] ptp_delay_req_message create_delay_req_message(const ptp_port_identity& source_port_identity) const {
        ptp_delay_req_message delay_req_message;
        delay_req_message.header.source_port_identity = source_port_identity;
        delay_req_message.header = sync_message_.header;
        delay_req_message.header.message_type = ptp_message_type::delay_req;
        delay_req_message.header.message_length =
            ptp_message_header::k_header_size + ptp_delay_req_message::k_message_size;
        delay_req_message.header.correction_field = 0;
        delay_req_message.origin_timestamp = sync_message_.origin_timestamp;
        return delay_req_message;
    }

private:
    ptp_sync_message sync_message_;
    std::optional<ptp_follow_up_message> follow_up_message_;
    ptp_timestamp t1_ {}, t2_ {}, t3_ {}, t4_ {};
};

}  // namespace rav
