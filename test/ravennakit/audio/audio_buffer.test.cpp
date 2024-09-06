/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2024 Owllab. All rights reserved.
 */

#include <catch2/catch_all.hpp>

#include <ravennakit/audio/audio_buffer.hpp>
#include <ravennakit/core/util.hpp>

namespace {
struct custom_sample_type {
    size_t channel_index;
    size_t sample_index;
};

template<class T>
void check_sample_values(const rav::audio_buffer<T>& buffer, const T& fill_value) {
    auto* const* audio_data = buffer.get_array_of_read_pointers();

    REQUIRE(buffer.num_channels() > 0);
    REQUIRE(buffer.num_samples() > 0);

    for (size_t ch = 0; ch < buffer.num_channels(); ch++) {
        for (size_t sample = 0; sample < buffer.num_samples(); sample++) {
            // The casting is a really quick and dirty way to avoid floating point comparison issues.
            rav::util::is_within(audio_data[ch][sample],fill_value, {});
        }
    }
}

}  // namespace

TEST_CASE("audio_buffer::audio_buffer()", "[audio_buffer]") {
    SECTION("Instantiate different buffer types to get an error when the buffer cannot be used for a particular type."
    ) {
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
        rav::audio_buffer<float> buffer {0, 0};

        // When the buffer holds no data, we expect nullptrs.
        REQUIRE(buffer.get_array_of_read_pointers() == nullptr);
        REQUIRE(buffer.get_array_of_write_pointers() == nullptr);
        REQUIRE(buffer.num_channels() == 0);
        REQUIRE(buffer.num_samples() == 0);
    }

    SECTION("Initial state with some buffers") {
        rav::audio_buffer<int> buffer(2, 5);
        REQUIRE(buffer.num_channels() == 2);
        REQUIRE(buffer.num_samples() == 5);
    }

    SECTION("Prepare buffer") {
        rav::audio_buffer<int> buffer;
        buffer.resize(2, 3);
        REQUIRE(buffer.num_channels() == 2);
        REQUIRE(buffer.num_samples() == 3);
    }

    SECTION("Construct and fill with value") {
        static constexpr size_t channel_sizes[] = {1, 2, 3, 512};
        static constexpr size_t sample_sizes[] = {1, 2, 128, 256};
        static constexpr int fill_value = 42;

        for (auto channel_size : channel_sizes) {
            for (auto sample_size : sample_sizes) {
                rav::audio_buffer buffer(channel_size, sample_size, fill_value);
                REQUIRE(buffer.num_channels() == channel_size);
                REQUIRE(buffer.num_samples() == sample_size);

                check_sample_values(buffer, fill_value);
            }
        }
    }
}

TEST_CASE("audio_buffer::set_sample()", "[audio_buffer]") {
    constexpr size_t num_channels = 3;
    constexpr size_t num_samples = 4;

    rav::audio_buffer<custom_sample_type> buffer(num_channels, num_samples);

    for (size_t ch = 0; ch < num_channels; ch++) {
        for (size_t sample = 0; sample < num_samples; sample++) {
            buffer.set_sample(ch, sample, custom_sample_type {ch, sample});
        }
    }

    auto* const* audio_data = buffer.get_array_of_read_pointers();

    for (size_t ch = 0; ch < num_channels; ch++) {
        for (size_t sample = 0; sample < num_samples; sample++) {
            REQUIRE(audio_data[ch][sample].channel_index == ch);
            REQUIRE(audio_data[ch][sample].sample_index == sample);
        }
    }
}

template<class T>
void test_audio_buffer_clear_for_type(T expected_cleared_value) {
    size_t num_channels = 3;
    size_t num_samples = 4;

    {
        rav::audio_buffer<T> buffer(num_channels, num_samples, 1);
        check_sample_values<T>(buffer, 1);
        buffer.clear();
        check_sample_values(buffer, expected_cleared_value);
    }

    {
        rav::audio_buffer<T> buffer(num_channels, num_samples, 1);
        check_sample_values<T>(buffer, 1);
        buffer.clear(expected_cleared_value);
        check_sample_values(buffer, expected_cleared_value);
    }

    {
        rav::audio_buffer<T> buffer(num_channels, num_samples, 1);
        check_sample_values<T>(buffer, 1);
        for (size_t ch = 0; ch < num_channels; ch++) {
            buffer.clear(ch, 0, num_samples);
        }
        check_sample_values(buffer, expected_cleared_value);
    }
}

TEST_CASE("audio_buffer::clear()", "[audio_buffer]") {
    test_audio_buffer_clear_for_type<float>(0.0f);
    test_audio_buffer_clear_for_type<double>(0.0);
    test_audio_buffer_clear_for_type<int8_t>(0);
    test_audio_buffer_clear_for_type<int16_t>(0);
    test_audio_buffer_clear_for_type<int32_t>(0);
    test_audio_buffer_clear_for_type<int64_t>(0);
    test_audio_buffer_clear_for_type<uint8_t>(128u);
    test_audio_buffer_clear_for_type<uint16_t>(32768u);
    test_audio_buffer_clear_for_type<uint32_t>(2147483648u);
    test_audio_buffer_clear_for_type<uint64_t>(9223372036854775808u);
}
