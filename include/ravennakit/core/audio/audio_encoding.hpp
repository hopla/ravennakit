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
#include <cstdint>

namespace rav {

enum class audio_encoding {
    undefined,
    pcm_s8,
    pcm_u8,
    pcm_s16le,
    pcm_s16be,
    pcm_s24le,
    pcm_s24be,
    pcm_s32le,
    pcm_s32be,
    pcm_float,
    pcm_double,
};

/**
 * @return The number of bytes per sample.
 */
inline uint8_t audio_encoding_bytes_per_sample(const audio_encoding encoding) {
    switch (encoding) {
        case audio_encoding::pcm_s8:
        case audio_encoding::pcm_u8:
            return 1;
        case audio_encoding::pcm_s16le:
        case audio_encoding::pcm_s16be:
            return 2;
        case audio_encoding::pcm_s24le:
        case audio_encoding::pcm_s24be:
            return 3;
        case audio_encoding::pcm_s32le:
        case audio_encoding::pcm_s32be:
        case audio_encoding::pcm_float:
            return 4;
        case audio_encoding::pcm_double:
            return 8;
        case audio_encoding::undefined:
        default:
            return 0;
    }
}

/**
 * @return The ground value for the encoding.
 */
inline uint8_t audio_encoding_ground_value(const audio_encoding encoding) {
    switch (encoding) {
        case audio_encoding::pcm_u8:
            return 0x80;
        case audio_encoding::pcm_s8:
        case audio_encoding::pcm_s16le:
        case audio_encoding::pcm_s16be:
        case audio_encoding::pcm_s24le:
        case audio_encoding::pcm_s24be:
        case audio_encoding::pcm_s32le:
        case audio_encoding::pcm_s32be:
        case audio_encoding::pcm_float:
        case audio_encoding::pcm_double:
        case audio_encoding::undefined:
        default:
            return 0;
    }
}

inline const char* audio_encoding_to_string(const audio_encoding encoding) {
    switch (encoding) {
        case audio_encoding::undefined:
            return "undefined";
        case audio_encoding::pcm_s8:
            return "pcm_s8";
        case audio_encoding::pcm_u8:
            return "pcm_u8";
        case audio_encoding::pcm_s16le:
            return "pcm_s16le";
        case audio_encoding::pcm_s16be:
            return "pcm_s16be";
        case audio_encoding::pcm_s24le:
            return "pcm_s24le";
        case audio_encoding::pcm_s24be:
            return "pcm_s24be";
        case audio_encoding::pcm_s32le:
            return "pcm_s32le";
        case audio_encoding::pcm_s32be:
            return "pcm_s32be";
        case audio_encoding::pcm_float:
            return "pcm_float";
        case audio_encoding::pcm_double:
            return "pcm_double";
        default:
            return "unknown";
    }
}

}  // namespace rav
