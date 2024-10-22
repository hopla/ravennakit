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

#include "wav_audio_format.data.cpp"
#include "ravennakit/audio/formats/wav_audio_format.hpp"
#include "ravennakit/containers/byte_stream.hpp"

TEST_CASE("wav_audio_format | Read wav file", "[wav_audio_format]") {
    REQUIRE(sin_1ms_wav.size() == 1808);

    rav::byte_stream stream(sin_1ms_wav);
    REQUIRE(stream.size().value() == 1808);

    rav::wav_audio_format::reader reader(stream);
    REQUIRE(reader.format() == rav::wav_audio_format::format_code::pcm);
    REQUIRE(reader.num_channels() == 2);
    REQUIRE(reader.sample_rate() == 44100);
}

TEST_CASE("wav_audio_format | Reading audio data from a file", "[wav_audio_format]") {}
