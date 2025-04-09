/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/core/net/interfaces/network_interface_list.hpp"

#include "ravennakit/core/log.hpp"

rav::NetworkInterfaceList::NetworkInterfaceList() {
    repopulate_with_system_interfaces();
}

rav::NetworkInterfaceList::NetworkInterfaceList(std::vector<NetworkInterface> interfaces) :
    interfaces_(std::move(interfaces)) {}

const rav::NetworkInterface* rav::NetworkInterfaceList::find_by_string(const std::string& search_string) const {
    if (search_string.empty()) {
        return nullptr;
    }

    // Match identifier
    for (auto& interface : interfaces_) {
        if (interface.get_identifier() == search_string) {
            return &interface;
        }
    }

    // Match display name
    for (auto& interface : interfaces_) {
        if (interface.get_display_name() == search_string) {
            return &interface;
        }
    }

    // Match description
    for (auto& interface : interfaces_) {
        if (interface.get_description() == search_string) {
            return &interface;
        }
    }

    // Match MAC address
    for (auto& interface : interfaces_) {
        const auto mac_addr = interface.get_mac_address();
        if (mac_addr && mac_addr->to_string() == search_string) {
            return &interface;
        }
    }

    // Match address
    for (auto& interface : interfaces_) {
        for (const auto& address : interface.get_addresses()) {
            if (address.to_string() == search_string) {
                return &interface;
            }
        }
    }

    return nullptr;
}

const rav::NetworkInterface* rav::NetworkInterfaceList::find_by_address(const asio::ip::address& address) const {
    for (auto& interface : interfaces_) {
        for (const auto& addr : interface.get_addresses()) {
            if (addr == address) {
                return &interface;
            }
        }
    }
    return nullptr;
}

void rav::NetworkInterfaceList::repopulate_with_system_interfaces() {
    auto result = NetworkInterface::get_all();
    if (!result) {
        RAV_ERROR("Failed to get network interfaces: {}", result.error());
        return;
    }
    interfaces_ = std::move(result.value());
}

const std::vector<rav::NetworkInterface>& rav::NetworkInterfaceList::interfaces() const {
    return interfaces_;
}

const rav::NetworkInterfaceList& rav::NetworkInterfaceList::get_system_interfaces(const bool force_refresh) {
    static NetworkInterfaceList instance(NetworkInterface::get_all().value());
    static std::chrono::steady_clock::time_point last_refresh_time = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    if (force_refresh || std::chrono::steady_clock::now() > last_refresh_time + k_ttl) {
        instance.repopulate_with_system_interfaces();
        last_refresh_time = now;
    }
    return instance;
}
