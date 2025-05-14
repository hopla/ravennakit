/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#include "ravennakit/nmos/nmos_node.hpp"

#include "ravennakit/nmos/models/nmos_error.hpp"

#include <boost/json/serialize.hpp>

namespace {

void set_error_response(
    const boost::beast::http::status status, const std::string& error, const std::string& debug,
    boost::beast::http::response<boost::beast::http::string_body>& res
) {
    res.result(status);
    res.set("Content-Type", "application/json");
    res.body() =
        boost::json::serialize(boost::json::value_from(rav::nmos::Error {static_cast<unsigned>(status), error, debug}));
    res.prepare_payload();
}

std::optional<rav::nmos::ApiVersion>
get_version_from_parameters(const rav::PathMatcher::Parameters& params, const std::string_view param_name = "version") {
    const auto version_str = params.get(param_name);
    if (version_str == nullptr) {
        return std::nullopt;
    }
    return rav::nmos::ApiVersion::from_string(*version_str);
}

void set_default_headers(rav::HttpServer::Response& res) {
    res.set("Content-Type", "application/json");
    res.set("Access-Control-Allow-Origin", "*");
    res.set("Access-Control-Allow-Methods", "GET, PUT, POST, PATCH, HEAD, OPTIONS, DELETE");
    res.set("Access-Control-Allow-Headers", "Content-Type, Accept");
    res.set("Access-Control-Max-Age", "3600");
}

}  // namespace

std::array<rav::nmos::ApiVersion, 2> rav::nmos::Node::k_supported_api_versions = {{
    ApiVersion::v1_2(),
    ApiVersion::v1_3(),
}};

boost::system::result<void, rav::nmos::Node::Error> rav::nmos::Node::Configuration::validate() const {
    if (operation_mode == OperationMode::registered_p2p) {
        if (discover_mode == DiscoverMode::dns || discover_mode == DiscoverMode::mdns) {
            return {};
        }
        // Unicast DNS and manual mode are not valid in registered_p2p mode because they are not valid for p2p
        return Error::incompatible_discover_mode;
    }

    if (operation_mode == OperationMode::registered) {
        if (discover_mode == DiscoverMode::dns || discover_mode == DiscoverMode::udns
            || discover_mode == DiscoverMode::mdns) {
            return {};
        }
        if (discover_mode == DiscoverMode::manual) {
            if (registry_address.empty()) {
                return Error::invalid_registry_address;
            }
            return {};
        }
        return Error::incompatible_discover_mode;
    }

    if (operation_mode == OperationMode::p2p) {
        if (discover_mode == DiscoverMode::mdns) {
            return {};
        }
        // Unicast DNS and manual mode are not valid in p2p mode
        return Error::incompatible_discover_mode;
    }

    RAV_ASSERT_FALSE("Should not have reached this line");
    return {};
}

rav::nmos::Node::Node(boost::asio::io_context& io_context) : http_server_(io_context) {
    http_server_.get("/", [this](const HttpServer::Request&, HttpServer::Response& res, PathMatcher::Parameters&) {
        res.result(boost::beast::http::status::ok);
        set_default_headers(res);
        res.body() = boost::json::serialize(boost::json::array({"x-nmos/"}));
        res.prepare_payload();
    });

    http_server_.get(
        "/x-nmos",
        [this](const HttpServer::Request&, HttpServer::Response& res, PathMatcher::Parameters&) {
            res.result(boost::beast::http::status::ok);
            set_default_headers(res);
            res.body() = boost::json::serialize(boost::json::array({"node/"}));
            res.prepare_payload();
        }
    );

    http_server_.get(
        "/x-nmos/node",
        [this](const HttpServer::Request&, HttpServer::Response& res, PathMatcher::Parameters&) {
            res.result(boost::beast::http::status::ok);
            set_default_headers(res);

            boost::json::array versions;
            for (const auto& version : k_supported_api_versions) {
                versions.push_back({fmt::format("{}/", version.to_string())});
            }

            res.body() = boost::json::serialize(versions);
            res.prepare_payload();
        }
    );

    http_server_.get(
        "/x-nmos/node/{version}",
        [this](const HttpServer::Request&, HttpServer::Response& res, const PathMatcher::Parameters& params) {
            const auto version = get_version_from_parameters(params);
            if (!version) {
                set_error_response(
                    boost::beast::http::status::bad_request, "Invalid API version",
                    "Failed to parse a valid version in the form of vMAJOR.MINOR", res
                );
                return;
            }

            res.result(boost::beast::http::status::ok);
            set_default_headers(res);
            res.body() = boost::json::serialize(
                boost::json::array({"self/", "sources/", "flows/", "devices/", "senders/", "receivers/"})
            );
            res.prepare_payload();
        }
    );

    http_server_.get("/**", [this](const HttpServer::Request&, HttpServer::Response& res, PathMatcher::Parameters&) {
        set_error_response(boost::beast::http::status::not_found, "Not found", "No matching route", res);
    });
}

boost::system::result<void> rav::nmos::Node::start(const std::string_view bind_address, const uint16_t port) {
    return http_server_.start(bind_address, port);
}

void rav::nmos::Node::stop() {
    http_server_.stop();
}

boost::asio::ip::tcp::endpoint rav::nmos::Node::get_local_endpoint() const {
    return http_server_.get_local_endpoint();
}

std::ostream& rav::nmos::operator<<(std::ostream& os, const Node::Error error) {
    switch (error) {
        case Node::Error::incompatible_discover_mode:
            os << "incompatible_discover_mode";
            break;
        case Node::Error::invalid_registry_address:
            os << "invalid_registry_address";
            break;
    }
    return os;
}

std::ostream& rav::nmos::operator<<(std::ostream& os, const Node::OperationMode operation_mode) {
    switch (operation_mode) {
        case Node::OperationMode::registered_p2p:
            os << "registered_p2p";
            break;
        case Node::OperationMode::registered:
            os << "registered";
            break;
        case Node::OperationMode::p2p:
            os << "p2p";
            break;
    }
    return os;
}

std::ostream& rav::nmos::operator<<(std::ostream& os, const Node::DiscoverMode discover_mode) {
    switch (discover_mode) {
        case Node::DiscoverMode::dns:
            os << "dns";
            break;
        case Node::DiscoverMode::udns:
            os << "udns";
            break;
        case Node::DiscoverMode::mdns:
            os << "mdns";
            break;
        case Node::DiscoverMode::manual:
            os << "manual";
            break;
    }
    return os;
}
