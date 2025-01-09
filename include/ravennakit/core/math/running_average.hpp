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

#include <type_traits>

namespace rav {

/**
 * A running average.
 * @tparam T The floating-point type of the average.
 */
template<class T = double>
class running_average {
public:
    static_assert(std::is_floating_point_v<T>, "T must be a floating-point type");

    /**
     * Adds a new value to the running average.
     * @param value The value to add.
     */
    void add(double value) {
        count_++;
        average_ += (value - average_) / count_;
    }

    /**
     * Adds a new value to the running average.
     * @tparam U The type of the value to add.
     * @param value The value to add.
     */
    template<class U>
    void add(U value) {
        add(static_cast<double>(value));
    }

    /**
     * @return The current average.
     */
    T average() const {
        return average_;
    }

    /**
     * @returns The number of values added to the running average.
     */
    [[nodiscard]] size_t count() const {
        return count_;
    }

    /**
     * Resets the running average.
     */
    void reset() {
        count_ = 0;
        average_ = 0;
    }

private:
    T average_ {};
    size_t count_ {};
};

}  // namespace rav
