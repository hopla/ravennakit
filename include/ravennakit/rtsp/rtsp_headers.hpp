/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#pragma once

#include "ravennakit/core/string.hpp"

#include <string>
#include <vector>

namespace rav {

class rtsp_headers {
public:
    struct header {
        std::string name;
        std::string value;
    };

    /**
     * Finds a header by name and returns its value.
     * @param name The name of the header.
     * @returns The value of the header if found, otherwise nullptr.
     */
    [[nodiscard]] const std::string* get_header_value(const std::string& name) const {
        for (const auto& header : headers_) {
            if (header.name == name) {
                return &header.value;
            }
        }
        return nullptr;
    }

    /**
     * @returns Tries to find the Content-Length header and returns its value as integer.
     */
    [[nodiscard]] std::optional<long> get_content_length() const {
        if (const std::string* content_length = get_header_value("Content-Length"); content_length) {
            return rav::ston<long>(*content_length);
        }
        return std::nullopt;
    }

    header& operator[] (const size_t index) {
        return headers_[index];
    }

    [[nodiscard]] const header& operator[] (const size_t index) const {
        return headers_[index];
    }

    [[nodiscard]] const std::vector<header>& get_headers() const {
        return headers_;
    }

    void clear() {
        headers_.clear();
    }

    [[nodiscard]] bool empty() const {
        return headers_.empty();
    }

    [[nodiscard]] size_t size() const {
        return headers_.size();
    }

    void push_back(header header) {
        headers_.push_back(std::move(header));
    }

    header& emplace_back() {
        return headers_.emplace_back();
    }

    header& emplace_back(header&& header) {
        return headers_.emplace_back(std::move(header));
    }

    header& back() {
        return headers_.back();
    }

private:
    std::vector<header> headers_;
};

}
