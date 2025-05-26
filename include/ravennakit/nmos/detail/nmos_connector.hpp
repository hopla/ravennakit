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

#include "nmos_api_version.hpp"
#include "nmos_discover_mode.hpp"
#include "nmos_operating_mode.hpp"
#include "nmos_registry_browser.hpp"
#include "ravennakit/core/net/http/http_client.hpp"
#include "ravennakit/core/util/safe_function.hpp"
#include "ravennakit/core/util/todo.hpp"

#include <boost/asio.hpp>
#include <boost/json/serialize.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace rav::nmos {

/**
 * Finds and maintains a connection to an NMOS registry, or finds and connects to nodes when p2p is enabled.
 */
class Connector {
  public:
    static constexpr auto k_default_timeout = std::chrono::milliseconds(1000);
    static constexpr uint8_t k_max_failed_heartbeats = 5;
    static constexpr auto k_heartbeat_interval = std::chrono::seconds(5);

    SafeFunction<void(bool connected)> on_connected_status_changed;

    explicit Connector(boost::asio::io_context& io_context) :
        registry_browser_(io_context), http_client_(io_context, k_default_timeout), timer_(io_context) {}

    void start(
        const OperationMode operation_mode, const DiscoverMode discover_mode, const ApiVersion api_version,
        const std::string& registry_address
    ) {
        operation_mode_ = operation_mode;
        api_version_ = api_version;

        timer_.stop();

        // Registry browser
        if ((operation_mode == OperationMode::registered || operation_mode == OperationMode::registered_p2p)
            && discover_mode != DiscoverMode::manual) {
            registry_browser_.on_registry_discovered.reset();
            registry_browser_.start(discover_mode, api_version);

            timer_.once(k_default_timeout, [this] {
                // Subscribe to future registry discoveries
                registry_browser_.on_registry_discovered = [this](const dnssd::ServiceDescription& desc) {
                    handle_discovered_registry(desc);
                };
                if (const auto reg = registry_browser_.find_most_suitable_registry()) {
                    connect_to_registry_async(*reg);
                } else if (operation_mode_ == OperationMode::registered_p2p) {
                    set_p2p_fallback_active(true);
                }
            });
        } else {
            registry_browser_.stop();
        }

        // Node browser
        if ((operation_mode == OperationMode::registered_p2p || operation_mode == OperationMode::p2p)
            && discover_mode != DiscoverMode::manual) {
            // TODO: Start Node browser
        } else {
            // TODO: Stop Node browser
        }

        if (discover_mode == DiscoverMode::manual) {
            if (registry_address.empty()) {
                RAV_ERROR("Registry address is empty");
                return;
            }

            const boost::urls::url_view url(registry_address);
            // connect_to_registry_async(url.host(), url.port_number());
            // TODO: connect to registry using the provided address
        }
    }

    void stop() {
        timer_.stop();
        registry_browser_.stop();
    }

    void post_async(
        const std::string_view target, std::string body, HttpClient::CallbackType callback,
        const std::string_view content_type = "application/json"
    ) {
        http_client_.post_async(target, std::move(body), std::move(callback), content_type);
    }

    /**
     * Clears all scheduled requests if there are any. Otherwise, this function has no effect.
     */
    void cancel_outstanding_requests() {
        http_client_.cancel_outstanding_requests();
    }

    void try_reconnect() {
        reconnect_to_registry_async();
    }

  private:
    OperationMode operation_mode_ = OperationMode::registered_p2p;
    ApiVersion api_version_ = ApiVersion::v1_3();
    bool p2p_fallback_active_ = false;
    bool is_connected_to_registry_ = false;
    std::optional<std::string> registry_fullname_;
    RegistryBrowser registry_browser_;
    HttpClient http_client_;
    AsioTimer timer_;

    void handle_discovered_registry(const dnssd::ServiceDescription& desc) {
        RAV_INFO("Discovered NMOS registry: {}", desc.name);

        if (operation_mode_ == OperationMode::registered_p2p || operation_mode_ == OperationMode::registered) {
            if (p2p_fallback_active_) {
                set_p2p_fallback_active(false);
            } else {
                if (desc.fullname == registry_fullname_) {
                    RAV_TRACE("Already connected to the discovered registry: {}", desc.fullname);
                    return;  // Already connected to this registry
                }
            }
            connect_to_registry_async(desc);
        }
    }

    void connect_to_registry_async(const dnssd::ServiceDescription& desc) {
        if (desc.fullname == registry_fullname_) {
            RAV_TRACE("Already connected to the discovered registry: {}", desc.fullname);
            return;  // Already connected to this registry
        }
        RAV_INFO("Connecting to NMOS registry: {}", desc.fullname);
        http_client_.set_host(string_remove_suffix(desc.host_target, "."), std::to_string(desc.port));
        registry_fullname_ = desc.fullname;
        reconnect_to_registry_async();
    }

    void reconnect_to_registry_async() {
        http_client_.get_async("/", [this](const boost::system::result<http::response<http::string_body>>& result) {
            set_connected(result.has_value() && result.value().result() == http::status::ok);
        });
    }

    void set_connected(bool is_connected) {
        if (std::exchange(is_connected_to_registry_, is_connected) != is_connected) {
            on_connected_status_changed(is_connected);
        }
    }

    void set_p2p_fallback_active(const bool active) {
        if (p2p_fallback_active_ == active) {
            return;  // No change in state
        }
        if (active) {
            RAV_INFO("Falling back to p2p mode due to no suitable registry found");
        } else {
            RAV_INFO("Disabling p2p mode because a suitable registry was found");
        }
        p2p_fallback_active_ = active;
    }
};

}  // namespace rav::nmos
