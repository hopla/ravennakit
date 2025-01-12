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
     * Adds a new value to the sliding window average, recalculates the median and returns the updated median.
     * @param value The value to add.
     * @return The updated median of the values in the window.
     */
    double add(const double value) {
        window_.push_back(value);
        median_buffer_.clear();
        for (auto& e : window_) {
            median_buffer_.push_back(e);
        }
        if (median_buffer_.empty()) {
            median_ = 0.0;
            return median_;
        }
        std::sort(median_buffer_.begin(), median_buffer_.end());
        const size_t n = median_buffer_.size();
        if (n % 2 == 1) {
            median_ = median_buffer_[n / 2]; // Odd: return the middle element
            return median_;
        }
        median_ = (median_buffer_[n / 2 - 1] + median_buffer_[n / 2]) / 2.0; // Even: return the average of the two middle elements
        return median_;
    }

    /**
     * @return The median of the values in the window.
     */
    [[nodiscard]] double median() const {
        return median_;
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
    [[nodiscard]] bool is_outlier(const double value, const double threshold) const {
        const auto diff = std::abs(value - median_);
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
    double median_ {}; // Latest median value
};

}  // namespace rav
