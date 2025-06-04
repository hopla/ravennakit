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

#include <fmt/ostream.h>

namespace rav::nmos {

/**
 * The mode of discovery to find an NMOS registry, for both registry and peer-to-peer discovery.
 */
enum class DiscoverMode {
    /// The node will use both multicast and unicast DNS to discover registries.
    dns,
    /// The node will use unicast DNS to discover registries.
    udns,
    /// The node will use multicast DNS to discover registries or other nodes.
    mdns,
    /// The node will connect to the manually specified address and port and not discover anything.
    manual,
};

/// Overload the output stream operator for the Node::DiscoverMode enum class
inline std::ostream& operator<<(std::ostream& os, const DiscoverMode discover_mode) {
    switch (discover_mode) {
        case DiscoverMode::dns:
            os << "dns";
            break;
        case DiscoverMode::udns:
            os << "udns";
            break;
        case DiscoverMode::mdns:
            os << "mdns";
            break;
        case DiscoverMode::manual:
            os << "manual";
            break;
    }
    return os;
}

}  // namespace rav::nmos

/// Make Node::DiscoverMode printable with fmt
template<>
struct fmt::formatter<rav::nmos::DiscoverMode>: ostream_formatter {};
