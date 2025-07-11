/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/rtp/detail/rtp_receiver.hpp"
#include "ravennakit/rtp/detail/rtp_receiver.hpp"

#include "ravennakit/core/log.hpp"
#include "ravennakit/rtp/rtcp_packet_view.hpp"
#include "ravennakit/rtp/detail/rtp_receiver.hpp"

#include "ravennakit/core/util/subscriber_list.hpp"
#include "ravennakit/rtp/rtp_packet_view.hpp"
#include "ravennakit/core/util/tracy.hpp"

#include <fmt/core.h>
#include "ravennakit/core/expected.hpp"
#include "ravennakit/core/util/defer.hpp"

#include <utility>

#if RAV_APPLE
    #define IP_RECVDSTADDR_PKTINFO IP_RECVDSTADDR
#else
    #define IP_RECVDSTADDR_PKTINFO IP_PKTINFO
#endif

#if RAV_WINDOWS
typedef BOOL(PASCAL* LPFN_WSARECVMSG)(
    SOCKET s, LPWSAMSG lpMsg, LPDWORD lpNumberOfBytesRecvd, LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);
#endif

namespace {

bool setup_socket(boost::asio::ip::udp::socket& socket, const uint16_t port) {
    const auto endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), port);

    try {
        socket.open(boost::asio::ip::udp::v4());
        socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        socket.bind(endpoint);
        socket.non_blocking(true);
        socket.set_option(boost::asio::detail::socket_option::integer<IPPROTO_IP, IP_RECVDSTADDR_PKTINFO>(1));
    } catch (const std::exception& e) {
        RAV_ERROR("Failed to setup receive socket: {}", e.what());
        socket.close();
        return false;
    }

    return true;
}

boost::asio::ip::udp::socket*
get_or_create_socket(rav::rtp::Receiver3& receiver, const uint16_t port, boost::asio::io_context& io_context) {
    // Try to find existing socket
    for (auto& ctx : receiver.sockets) {
        if (ctx.port == port) {
            return &ctx.socket;
        }
    }

    // Try to reuse existing socket slot
    for (auto& ctx : receiver.sockets) {
        if (ctx.state.load(std::memory_order_acquire) != rav::rtp::Receiver3::State::available) {
            continue;  // Slot not available, try next one
        }
        if (!setup_socket(ctx.socket, port)) {
            return nullptr;
        }
        ctx.port = port;
        ctx.state.store(rav::rtp::Receiver3::State::ready, std::memory_order_release);
        return &ctx.socket;
    }

    // Try to add new socket
    if (receiver.sockets.size() >= decltype(receiver.sockets)::max_size()) {
        return nullptr;  // No space left
    }

    auto& it = receiver.sockets.emplace_back(io_context);
    if (!setup_socket(it.socket, port)) {
        return nullptr;
    }
    it.port = port;
    it.state.store(rav::rtp::Receiver3::State::ready, std::memory_order_release);
    return &it.socket;
}

uint32_t count_multicast_groups(
    rav::rtp::Receiver3& receiver, const boost::asio::ip::address_v4& multicast_group,
    const boost::asio::ip::address_v4& interface_address, const uint16_t port
) {
    RAV_ASSERT(multicast_group.is_multicast(), "Multicast group should be a multicast address");
    RAV_ASSERT(!interface_address.is_unspecified(), "Interface address should not be unspecified");
    RAV_ASSERT(!interface_address.is_multicast(), "Interface address should not be a multicast address");
    RAV_ASSERT(port > 0, "Port must be a non-zero value");

    uint32_t total = 0;
    for (size_t i = 0; i < receiver.interface_addresses.size(); i++) {
        if (receiver.interface_addresses[i] != interface_address) {
            continue;
        }
        for (auto& reader : receiver.readers) {
            if (reader.sessions[i].connection_address == multicast_group && reader.sessions[i].rtp_port == port) {
                total++;
            }
        }
    }
    return total;
}

bool setup_reader(
    rav::rtp::Receiver3& receiver, rav::rtp::Receiver3::Reader& reader, const rav::Id id,
    const rav::rtp::Receiver3::ArrayOfSessions& sessions, const rav::rtp::Receiver3::ArrayOfFilters& filters,
    boost::asio::io_context& io_context
) {
    reader.id = id;
    reader.sessions = sessions;
    reader.filters = filters;
    reader.fifo.consume_all([](const auto&) {});
    reader.receive_buffer.clear();

    for (size_t i = 0; i < reader.sessions.size(); ++i) {
        auto& session = reader.sessions[i];
        if (!session.valid()) {
            continue;
        }
        const auto endpoint = boost::asio::ip::udp::endpoint(session.connection_address, session.rtp_port);

        auto* socket = get_or_create_socket(receiver, endpoint.port(), io_context);
        if (socket == nullptr) {
            RAV_ERROR("Failed to create receive socket");
            continue;
        }

        auto& interface_address = receiver.interface_addresses[i];
        if (session.connection_address.is_multicast()) {
            if (!interface_address.is_unspecified()) {
                const auto count = count_multicast_groups(
                    receiver, session.connection_address.to_v4(), interface_address, session.rtp_port
                );
                // 1 because the reader being set up is also counted
                if (count == 1) {
                    if (!receiver.join_multicast_group(
                            *socket, session.connection_address.to_v4(), interface_address
                        )) {
                        RAV_ERROR("Failed to join multicast group");
                    }
                }
            }
        }
    }

    return true;
}

}  // namespace

rav::rtp::Receiver3::Receiver3() {
    join_multicast_group = [](boost::asio::ip::udp::socket& socket, const boost::asio::ip::address_v4& multicast_group,
                              const boost::asio::ip::address_v4& interface_address) {
        RAV_ASSERT(multicast_group.is_multicast(), "Multicast group should be a multicast address");
        RAV_ASSERT(!interface_address.is_unspecified(), "Interface address should not be unspecified");
        RAV_ASSERT(!interface_address.is_multicast(), "Interface address should not be a multicast address");

        boost::system::error_code ec;
        socket.set_option(boost::asio::ip::multicast::join_group(multicast_group, interface_address), ec);
        if (ec) {
            RAV_ERROR("Failed to join multicast group: {}", ec.message());
            return false;
        }
        return true;
    };

    leave_multicast_group = [](boost::asio::ip::udp::socket& socket, const boost::asio::ip::address_v4& multicast_group,
                               const boost::asio::ip::address_v4& interface_address) {
        RAV_ASSERT(multicast_group.is_multicast(), "Multicast group should be a multicast address");
        RAV_ASSERT(!interface_address.is_unspecified(), "Interface address should not be unspecified");
        RAV_ASSERT(!interface_address.is_multicast(), "Interface address should not be a multicast address");

        boost::system::error_code ec;
        socket.set_option(boost::asio::ip::multicast::leave_group(multicast_group, interface_address), ec);
        if (ec) {
            RAV_ERROR("Failed to leave multicast group: {}", ec.message());
            return false;
        }
        return true;
    };
}

void rav::rtp::Receiver3::set_interface_addresses(const ArrayOfAddresses& addresses) {
    for (size_t i = 0; i < interface_addresses.size(); i++) {
        if (interface_addresses[i] != addresses[i]) {
            if (!interface_addresses[i].is_unspecified()) {
                // Leave existing group (sessions[i], interface_addresses[i])
            }

            if (!addresses[i].is_unspecified()) {
                // Join group (sessions[i], addresses[i])
            }
        }
    }

    interface_addresses = addresses;
}

bool rav::rtp::Receiver3::add_reader(
    const Id id, const ArrayOfSessions& sessions, const ArrayOfFilters& filters, boost::asio::io_context& io_context
) {
    RAV_ASSERT(sessions.size() == filters.size(), "Should be equal");
    RAV_ASSERT(sessions.size() == interface_addresses.size(), "Should be equal");

    for (auto& stream : readers) {
        if (stream.id == id) {
            RAV_WARNING("A stream for given id already exists");
            return false;  // Stream already exists
        }
    }

    for (auto& reader : readers) {
        const auto guard = reader.rw_lock.lock_exclusive();
        if (!guard) {
            RAV_ERROR("Failed to exclusively lock reader");
            return false;
        }

        if (reader.id.is_valid()) {
            continue;
        }

        return setup_reader(*this, reader, id, sessions, filters, io_context);
    }

    if (readers.size() >= decltype(readers)::max_size()) {
        RAV_TRACE("No space available for stream");
        return false;  // No space left
    }

    // Open a new reader slot
    auto& reader = readers.emplace_back();
    const auto guard = reader.rw_lock.lock_exclusive();
    if (!guard) {
        RAV_ERROR("Failed to exclusively lock reader");
        return false;
    }

    return setup_reader(*this, reader, id, sessions, filters, io_context);
}

bool rav::rtp::Receiver3::remove_reader(const Id id) {
    for (auto& e : readers) {
        if (e.id == id) {
            const auto guard = e.rw_lock.lock_exclusive();
            if (!guard) {
                RAV_ERROR("Failed to exclusively lock reader");
                return false;
            }

            for (size_t i = 0; i < e.sessions.size(); ++i) {
                auto& session = e.sessions[i];
                if (session.valid()) {
                    if (session.connection_address.is_multicast()) {
                        // Leave multicast group, if this is the last one
                        const auto count = count_multicast_groups(
                            *this, session.connection_address.to_v4(), interface_addresses[i], session.rtp_port
                        );
                        if (count == 1) {
                            // TODO: Since this is the last instance of this multicast group, we have to leave it.
                        }
                    }
                }
            }

            e.id = {};
            e.sessions = {};
            e.filters = {};
            e.fifo.consume_all([](const auto&) {});
            e.receive_buffer.clear();

            return true;
        }
    }
    return false;
}

void rav::rtp::Receiver3::read_incoming_packets() {
    for (auto& ctx : sockets) {
        std::array<uint8_t, aes67::constants::k_mtu> receive_buffer {};

        if (ctx.state.load(std::memory_order_acquire) != State::ready) {
            continue;
        }

        if (!ctx.socket.is_open()) {
            continue;
        }

        if (!ctx.socket.available()) {
            continue;
        }

        boost::asio::ip::udp::endpoint src_endpoint;
        boost::asio::ip::udp::endpoint dst_endpoint;
        uint64_t recv_time = 0;
        boost::system::error_code ec;
        const auto bytes_received =
            receive_from_socket(ctx.socket, receive_buffer, src_endpoint, dst_endpoint, recv_time, ec);

        if (ec) {
            RAV_ERROR("Failed to receive from socket: {}", ec.message());
            continue;
        }

        if (bytes_received == 0) {
            continue;
        }

        PacketView view(receive_buffer.data(), bytes_received);
        if (!view.validate()) {
            continue;  // Invalid RTP packet
        }

        for (auto& reader : readers) {
            const auto guard = reader.rw_lock.try_lock_shared();
            if (!guard) {
                continue;  // Failed to lock which means it is being added or removed.
            }

            if (!reader.id.is_valid()) {
                continue;
            }

            bool valid_source = false;
            for (size_t i = 0; i < reader.sessions.size(); ++i) {
                if (reader.sessions[i].connection_address == dst_endpoint.address()
                    && reader.sessions[i].rtp_port == dst_endpoint.port()) {
                    if (reader.filters[i].is_valid_source(dst_endpoint.address(), src_endpoint.address())) {
                        valid_source = true;
                        break;
                    }
                }
            }

            if (!valid_source) {
                continue;
            }

            std::array<uint8_t, aes67::constants::k_mtu> packet {};
            std::memcpy(packet.data(), receive_buffer.data(), bytes_received);

            fmt::println(
                "{}:{} => {}:{}", src_endpoint.address().to_string(), src_endpoint.port(),
                dst_endpoint.address().to_string(), dst_endpoint.port()
            );

            if (!reader.fifo.push(packet)) {
                RAV_ERROR("Failed to push data to fifo");
            }
        }
    }

    for (auto& ctx : sockets) {
        if (ctx.state.load(std::memory_order_acquire) == State::should_be_closed) {
            ctx.socket.close();  // TODO: Can we defer this call to close to the maintenance thread?
            ctx.state.store(State::available, std::memory_order_release);
        }
    }
}

rav::rtp::Receiver::Receiver(UdpReceiver& udp_receiver) : udp_receiver_(udp_receiver) {}

bool rav::rtp::Receiver::subscribe(
    Subscriber* subscriber, const Session& session, const boost::asio::ip::address_v4& interface_address
) {
    auto* context = find_or_create_session_context(session, interface_address);

    if (context == nullptr) {
        RAV_WARNING("Failed to find or create new session context");
        return false;
    }

    RAV_ASSERT(context != nullptr, "Expecting valid session at this point");

    return context->add_subscriber(subscriber);
}

bool rav::rtp::Receiver::unsubscribe(const Subscriber* subscriber) {
    size_t count = 0;
    for (auto it = sessions_contexts_.begin(); it != sessions_contexts_.end();) {
        if ((*it)->remove_subscriber(subscriber)) {
            count++;
        }
        if ((*it)->get_subscriber_count() == 0) {
            it = sessions_contexts_.erase(it);
        } else {
            ++it;
        }
    }
    return count > 0;
}

rav::rtp::Receiver::SessionContext::SessionContext(
    UdpReceiver& udp_receiver, Session session, boost::asio::ip::address_v4 interface_address
) :
    udp_receiver_(udp_receiver), session_(std::move(session)), interface_address_(std::move(interface_address)) {
    RAV_ASSERT(!session_.connection_address.is_unspecified(), "Connection address should not be unspecified");
    RAV_ASSERT(!interface_address_.is_unspecified(), "Interface address should not be unspecified");
    RAV_ASSERT(!interface_address_.is_multicast(), "Interface address should not be multicast");
    subscribe_to_udp_receiver(interface_address_);
}

rav::rtp::Receiver::SessionContext::~SessionContext() {
    udp_receiver_.unsubscribe(this);
}

bool rav::rtp::Receiver::SessionContext::add_subscriber(Receiver::Subscriber* subscriber) {
    return subscribers_.add(subscriber);
}

bool rav::rtp::Receiver::SessionContext::remove_subscriber(const Receiver::Subscriber* subscriber) {
    return subscribers_.remove(subscriber);
}

size_t rav::rtp::Receiver::SessionContext::get_subscriber_count() const {
    return subscribers_.size();
}

const rav::rtp::Session& rav::rtp::Receiver::SessionContext::get_session() const {
    return session_;
}

const boost::asio::ip::address_v4& rav::rtp::Receiver::SessionContext::interface_address() const {
    return interface_address_;
}

void rav::rtp::Receiver::SessionContext::on_receive(const ExtendedUdpSocket::RecvEvent& event) {
    if (event.dst_endpoint.port() == session_.rtp_port) {
        handle_incoming_rtp_data(event);
    } else if (event.dst_endpoint.port() == session_.rtcp_port) {
        // TODO: Handle RTCP data
    } else {
        RAV_WARNING("Received data on unknown port: {}", event.dst_endpoint.port());
    }
}

void rav::rtp::Receiver::SessionContext::handle_incoming_rtp_data(const ExtendedUdpSocket::RecvEvent& event) {
    TRACY_ZONE_SCOPED;

    const PacketView packet(event.data, event.size);
    if (!packet.validate()) {
        RAV_WARNING("Invalid RTP packet received");
        return;
    }

    const RtpPacketEvent rtp_event {packet, session_, event.src_endpoint, event.dst_endpoint, event.recv_time};

    bool did_find_stream = false;

    for (const auto& ssrc : synchronization_sources_) {
        if (ssrc == packet.ssrc()) {
            did_find_stream = true;
        }
    }

    if (!did_find_stream) {
        auto ssrc = synchronization_sources_.emplace_back(packet.ssrc());
        RAV_TRACE(
            "Added new stream with SSRC {} from {}:{}", ssrc, event.src_endpoint.address().to_string(),
            event.src_endpoint.port()
        );
    }

    for (auto* s : subscribers_) {
        s->on_rtp_packet(rtp_event);
    }
}

void rav::rtp::Receiver::SessionContext::subscribe_to_udp_receiver(
    const boost::asio::ip::address_v4& interface_address
) {
    if (session_.connection_address.is_multicast()) {
        if (!udp_receiver_.subscribe(this, session_.connection_address.to_v4(), interface_address, session_.rtp_port)) {
            RAV_ERROR("Failed to subscribe to multicast RTP session {}", session_.to_string());
        }
        if (!udp_receiver_.subscribe(
                this, session_.connection_address.to_v4(), interface_address, session_.rtcp_port
            )) {
            RAV_ERROR("Failed to subscribe to multicast RTP session {}", session_.to_string());
        }
    } else {
        if (!udp_receiver_.subscribe(this, interface_address, session_.rtp_port)) {
            RAV_ERROR("Failed to subscribe to unicast RTP session {}", session_.to_string());
        }
        if (!udp_receiver_.subscribe(this, interface_address, session_.rtcp_port)) {
            RAV_ERROR("Failed to subscribe to unicast RTP session {}", session_.to_string());
        }
    }
}

rav::rtp::Receiver::SessionContext* rav::rtp::Receiver::find_session_context(
    const Session& session, const boost::asio::ip::address_v4& interface_address
) const {
    for (const auto& context : sessions_contexts_) {
        if (context->get_session() == session && context->interface_address() == interface_address) {
            return context.get();
        }
    }
    return nullptr;
}

rav::rtp::Receiver::SessionContext* rav::rtp::Receiver::create_new_session_context(
    const Session& session, const boost::asio::ip::address_v4& interface_address
) {
    // TODO: Disallow a port to be used in multiple sessions because when receiving RTP data we don't know which session
    // it belongs to.

    auto new_session = std::make_unique<SessionContext>(udp_receiver_, session, interface_address);
    const auto& it = sessions_contexts_.emplace_back(std::move(new_session));
    RAV_TRACE("New RTP session context created for: {}", session.to_string());
    return it.get();
}

rav::rtp::Receiver::SessionContext* rav::rtp::Receiver::find_or_create_session_context(
    const Session& session, const boost::asio::ip::address_v4& interface_address
) {
    auto context = find_session_context(session, interface_address);
    if (context == nullptr) {
        context = create_new_session_context(session, interface_address);
    }
    return context;
}
