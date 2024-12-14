/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/ptp/ptp_port.hpp"

#include "ravennakit/core/util.hpp"
#include "ravennakit/core/containers/buffer_view.hpp"
#include "ravennakit/ptp/ptp_profiles.hpp"
#include "ravennakit/ptp/messages/ptp_message.hpp"

namespace {
const auto k_ptp_multicast_address = asio::ip::make_address("224.0.1.129");
constexpr auto k_ptp_event_port = 319;
constexpr auto k_ptp_general_port = 320;
}  // namespace

rav::ptp_port::ptp_port(
    asio::io_context& io_context, const asio::ip::address& interface_address, const ptp_port_identity port_identity
) :
    event_socket_(io_context, asio::ip::address_v4(), k_ptp_event_port),
    general_socket_(io_context, asio::ip::address_v4(), k_ptp_general_port) {
    // Initialize the port data set
    port_ds_.port_identity = port_identity;
    port_ds_.port_state = ptp_state::initializing;

    subscriptions_.push_back(event_socket_.join_multicast_group(k_ptp_multicast_address, interface_address));
    subscriptions_.push_back(general_socket_.join_multicast_group(k_ptp_multicast_address, interface_address));

    auto handler = [this](const udp_sender_receiver::recv_event& event) {
        handle_recv_event(event);
    };

    event_socket_.start(handler);
    general_socket_.start(handler);
}

uint16_t rav::ptp_port::get_port_number() const {
    return port_ds_.port_identity.port_number;
}

void rav::ptp_port::assert_valid_state(const ptp_profile& profile) const {
    port_ds_.assert_valid_state(profile);
}

void rav::ptp_port::handle_recv_event(const udp_sender_receiver::recv_event& event) {
    const buffer_view data(event.data, event.size);
    auto header = ptp_message_header::from_data(data);
    if (!header) {
        RAV_TRACE("PTP Error: {}", static_cast<std::underlying_type_t<rav::ptp_error>>(header.error()));
        return;
    }

    switch (header->message_type) {
        case ptp_message_type::announce: {
            auto announce_message = ptp_announce_message::from_data(data.subview(ptp_message_header::k_header_size));
            if (!announce_message) {
                RAV_TRACE("PTP Error: {}", to_string(announce_message.error()));
            }
            RAV_TRACE("{} {}", header->to_string(), announce_message->to_string());
            break;
        }
        case ptp_message_type::sync:
            break;
        case ptp_message_type::delay_req:
            break;
        case ptp_message_type::p_delay_req:
            break;
        case ptp_message_type::p_delay_resp:
            break;
        case ptp_message_type::follow_up:
            break;
        case ptp_message_type::delay_resp:
            break;
        case ptp_message_type::p_delay_resp_follow_up:
            break;
        case ptp_message_type::signaling:
            break;
        case ptp_message_type::management:
            break;
        case ptp_message_type::reserved1:
        case ptp_message_type::reserved2:
        case ptp_message_type::reserved3:
        case ptp_message_type::reserved4:
        case ptp_message_type::reserved5:
        case ptp_message_type::reserved6:
        default: {
            RAV_WARNING("Unknown PTP message received: {}", to_string(header->message_type));
        }
    }
}

void rav::ptp_port::handle_announce_message(
    const ptp_message_header& header, const ptp_announce_message& announce_message, buffer_view<const uint8_t> tlvs
) {}
