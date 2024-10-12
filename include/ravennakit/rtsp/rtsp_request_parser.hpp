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

#include "rtsp_request.hpp"

namespace rav {

class rtsp_request_parser {
  public:
    enum class result {
        good,
        indeterminate,
        bad_method,
        bad_uri,
        bad_protocol,
        bad_version,
        bad_header,
        bad_end_of_headers
    };

    explicit rtsp_request_parser(rtsp_request& request) : request_(request) {}

    template<typename InputIterator>
    std::tuple<result, InputIterator> parse(InputIterator begin, InputIterator end) {
        RAV_ASSERT(begin < end, "Invalid input iterators");

        while (begin < end) {
            if (remaining_expected_data_ > 0) {
                auto data_size = std::min(remaining_expected_data_, static_cast<long>(end - begin));
                request_.data.insert(request_.data.end(), begin, begin + data_size);
                remaining_expected_data_ -= data_size;
                begin += data_size;
                if (remaining_expected_data_ > 0) {
                    return std::make_tuple(result::indeterminate, begin);
                }
                if (begin >= end) {
                    return std::make_tuple(result::good, begin); // Exhausted
                }
            }

            RAV_ASSERT(begin < end, "Expecting data available");
            RAV_ASSERT(remaining_expected_data_ == 0, "No remaining data should be expected at this point");

            result result = consume(*begin++);

            if (result == result::good) {
                if (const auto data_length = request_.get_content_length(); data_length.has_value()) {
                    remaining_expected_data_ = *data_length;
                } else {
                    remaining_expected_data_ = 0;
                }

                if (remaining_expected_data_ > 0) {
                    continue; // Next iteration to handle data.
                }

                RAV_ASSERT(begin == end, "Expecting no more data left at this point");

                return std::make_tuple(result, begin);
            }

            if (result != result::indeterminate) {
                return std::make_tuple(result, begin); // Error
            }
        }

        return std::make_tuple(result::indeterminate, begin);
    }

  private:
    enum class state {
        method_start,
        method,
        uri,
        rtsp_r,
        rtsp_t,
        rtsp_s,
        rtsp_p,
        rtsp_slash,
        version_major,
        version_dot,
        version_minor,
        expecting_newline_1,
        header_start,
        header_name,
        space_before_header_value,
        header_value,
    };

    rtsp_request& request_;
    state state_ {state::method_start};
    long remaining_expected_data_ {0};

    result consume(char c);

    /// Check if a byte is an HTTP character.
    static bool is_char(int c);

    /// Check if a byte is an HTTP control character.
    static bool is_ctl(int c);

    /// Check if a byte is defined as an HTTP tspecial character.
    static bool is_tspecial(int c);

    /// Check if a byte is a digit.
    static bool is_digit(int c);
};

}  // namespace rav
