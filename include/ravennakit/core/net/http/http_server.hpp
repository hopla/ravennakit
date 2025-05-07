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

#include <boost/asio.hpp>
#include <boost/system/result.hpp>

namespace rav {

class HttpServer {
  public:
    explicit HttpServer(boost::asio::io_context& io_context);

    boost::system::result<void> start(std::string_view host, uint16_t port);
    void stop();

  private:
    class Listener;
    std::shared_ptr<Listener> listener_;

    class ClientSession;
    std::vector<std::shared_ptr<ClientSession>> client_sessions_;

    boost::asio::io_context& io_context_;
};

}  // namespace rav
