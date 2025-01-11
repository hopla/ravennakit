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

#include "ravennakit/core/containers/ring_buffer.hpp"

#include <vector>
#include <algorithm>

namespace rav {

class sliding_median {
  public:
    explicit sliding_median(const size_t size) : window_(size), median_buffer_(size) {}

    /**
     * Adds a new value to the sliding window average.
     * @param value The value to add.
     */
    void add(const double value) {
        window_.push_back(value);
    }

    /**
     * @return The median of the values in the window.
     */
    double median() {
        median_buffer_.clear();
        for (auto& value : window_) {
            median_buffer_.push_back(value);
        }
        if (median_buffer_.empty()) {
            return 0.0;
        }
        std::sort(median_buffer_.begin(), median_buffer_.end());
        const size_t n = median_buffer_.size();
        if (n % 2 == 1) {
            return median_buffer_[n / 2]; // Odd: return the middle element
        }
        return (median_buffer_[n / 2 - 1] + median_buffer_[n / 2]) / 2.0; // Even: return the average of the two middle elements
    }

    /**
     * @return The number of values added to the sliding window average.
     */
    [[nodiscard]] size_t count() const {
        return window_.size();
    }

    /**
     * Checks if the current value is an outlier.
     * @param value The value to check.
     * @param threshold The threshold for the outlier check as ration
     * @return True if the current value is an outlier.
     */
    bool is_outlier(const double value, const double threshold) {
        const auto current_median = median();
        const auto diff = std::abs(value - current_median);
        return diff > threshold;
    }

    /**
     * Resets the sliding window average.
     */
    void reset() {
        window_.clear();
        median_buffer_.clear();
    }

  private:
    ring_buffer<double> window_;
    std::vector<double> median_buffer_; // Used for median calculations
};

}  // namespace rav
