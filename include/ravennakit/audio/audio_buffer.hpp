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

#include <vector>

namespace rav {

template<class T>
class audio_buffer {
  public:
    audio_buffer() = default;

    audio_buffer(const size_t num_channels, const size_t num_samples) {
        prepare(num_channels, num_samples);
    }

    audio_buffer(const audio_buffer& other) = delete;
    audio_buffer(audio_buffer&& other) noexcept = delete;

    audio_buffer& operator=(const audio_buffer& other) = delete;
    audio_buffer& operator=(audio_buffer&& other) noexcept = delete;

    void prepare(size_t num_channels, const size_t num_samples) {
        if (num_channels == 0 || num_samples == 0) {
            data_.clear();
            channels_.clear();
            return;
        }

        data_.resize(num_channels * num_samples, {});
        channels_.resize(num_channels);

        // Update channel pointers.
        for (size_t i = 0; i < num_channels; ++i) {
            channels_[i] = data_.data() + i * num_samples;
        }
    }

    const T* const* get_array_of_read_pointers() {
        return channels_.empty() ? nullptr : channels_.data();
    }

    T* const* get_array_of_write_pointers() {
        return channels_.empty() ? nullptr : channels_.data();
    }

  private:
    /// Holds the non-interleaved audio data (each channel consecutive).
    std::vector<T> data_;

    /// Holds pointers to the beginning of each channel.
    std::vector<T*> channels_;
};

}  // namespace rav
