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

namespace rav::nmos {

/**
 * The mode of operation for the NMOS node.
 */
enum class OperationMode {
    /// The node registers itself with a registry or falls back to peer-to-peer discovery if no registry is
    /// available. When a registry comes online during p2p operation, the node will switch to registered mode.
    registered_p2p,
    /// The node registers itself with a registry and does not use peer-to-peer discovery.
    registered,
    /// The node only uses peer-to-peer discovery and does not register with a registry.
    p2p,
};

/// Overload the output stream operator for the Node::Error enum class
inline std::ostream& operator<<(std::ostream& os, const OperationMode operation_mode) {
    switch (operation_mode) {
        case OperationMode::registered_p2p:
            os << "registered_p2p";
            break;
        case OperationMode::registered:
            os << "registered";
            break;
        case OperationMode::p2p:
            os << "p2p";
            break;
    }
    return os;
}

}  // namespace rav::nmos

/// Make Node::OperationMode printable with fmt
template<>
struct fmt::formatter<rav::nmos::OperationMode>: ostream_formatter {};
