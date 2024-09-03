/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/core/result.hpp"

const char* rav::result::what() const {
    if (std::get_if<no_error>(&error_) != nullptr) {
        return "no error";
    }

    if (auto* error = std::get_if<rav::error>(&error_)) {
        return error_to_description(*error);
    }

    if (auto* error_event = std::get_if<uvw::error_event>(&error_)) {
        return error_event->what();
    }

    return "n/a";
}

const char* rav::result::name() const {
    if (std::get_if<no_error>(&error_) != nullptr) {
        return "no error";
    }

    if (auto* error = std::get_if<rav::error>(&error_)) {
        return error_to_name(*error);
    }

    if (auto* error_event = std::get_if<uvw::error_event>(&error_)) {
        return error_event->name();
    }

    return "n/a";
}
