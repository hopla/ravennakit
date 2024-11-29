/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/sdp/detail/sdp_connection_info.hpp"

#include "ravennakit/core/string_parser.hpp"
#include "ravennakit/sdp/detail/sdp_constants.hpp"

rav::sdp::connection_info_field::parse_result<rav::sdp::connection_info_field>
rav::sdp::connection_info_field::parse_new(const std::string_view line) {
    string_parser parser(line);

    if (!parser.skip("c=")) {
        return parse_result<connection_info_field>::err("connection: expecting 'c='");
    }

    connection_info_field info;

    // Network type
    if (const auto network_type = parser.split(' ')) {
        if (*network_type == sdp::k_sdp_inet) {
            info.network_type = sdp::netw_type::internet;
        } else {
            return parse_result<connection_info_field>::err("connection: invalid network type");
        }
    } else {
        return parse_result<connection_info_field>::err("connection: failed to parse network type");
    }

    // Address type
    if (const auto address_type = parser.split(' ')) {
        if (*address_type == sdp::k_sdp_ipv4) {
            info.address_type = sdp::addr_type::ipv4;
        } else if (*address_type == sdp::k_sdp_ipv6) {
            info.address_type = sdp::addr_type::ipv6;
        } else {
            return parse_result<connection_info_field>::err("connection: invalid address type");
        }
    } else {
        return parse_result<connection_info_field>::err("connection: failed to parse address type");
    }

    // Address
    if (const auto address = parser.split('/')) {
        info.address = *address;
    }

    if (parser.exhausted()) {
        return parse_result<connection_info_field>::ok(std::move(info));
    }

    // Parse optional ttl and number of addresses
    if (info.address_type == sdp::addr_type::ipv4) {
        if (auto ttl = parser.read_int<int32_t>()) {
            info.ttl = *ttl;
        } else {
            return parse_result<connection_info_field>::err("connection: failed to parse ttl for ipv4 address");
        }
        if (parser.skip('/')) {
            if (auto num_addresses = parser.read_int<int32_t>()) {
                info.number_of_addresses = *num_addresses;
            } else {
                return parse_result<connection_info_field>::err(
                    "connection: failed to parse number of addresses for ipv4 address"
                );
            }
        }
    } else if (info.address_type == sdp::addr_type::ipv6) {
        if (auto num_addresses = parser.read_int<int32_t>()) {
            info.number_of_addresses = *num_addresses;
        } else {
            return parse_result<connection_info_field>::err(
                "connection: failed to parse number of addresses for ipv4 address"
            );
        }
    }

    if (!parser.exhausted()) {
        return parse_result<connection_info_field>::err("connection: unexpected characters at end of line");
    }

    return parse_result<connection_info_field>::ok(std::move(info));
}
