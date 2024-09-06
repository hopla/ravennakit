/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include "ravennakit/audio/audio_buffer.hpp"

#include <catch2/catch_all.hpp>

TEST_CASE("audio_buffer::audio_buffer()", "[audio_buffer]") {
    SECTION("Instantiate different buffer types to get an error when the buffer cannot be used for a particular type.") {
        { rav::audio_buffer<float> buffer; }
        { rav::audio_buffer<double> buffer; }

        { rav::audio_buffer<int8_t> buffer; }
        { rav::audio_buffer<int16_t> buffer; }
        { rav::audio_buffer<int32_t> buffer; }
        { rav::audio_buffer<int64_t> buffer; }

        { rav::audio_buffer<uint8_t> buffer; }
        { rav::audio_buffer<uint16_t> buffer; }
        { rav::audio_buffer<uint32_t> buffer; }
        { rav::audio_buffer<uint64_t> buffer; }
    }

    SECTION("Empty buffer state") {
        rav::audio_buffer<float> buffer{0, 0};

        // When the buffer holds no data, we expect nullptrs.
        REQUIRE(buffer.get_array_of_read_pointers() == nullptr);
        REQUIRE(buffer.get_array_of_write_pointers() == nullptr);
    }
}
