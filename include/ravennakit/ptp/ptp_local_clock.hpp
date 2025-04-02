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

#include "ravennakit/core/tracy.hpp"
#include "ravennakit/core/chrono/high_resolution_clock.hpp"
#include "types/ptp_timestamp.hpp"

namespace rav::ptp {

/**
 * Represents a local clock as specified in IEEE 1588-2019.
 */
class LocalClock {
public:
    virtual ~LocalClock() = default;

    /**
     * @return The best estimate of 'now' in the timescale of the grand master clock.
     */
    [[nodiscard]] virtual Timestamp now() const = 0;

    /**
     * Adjusts the correction of this clock by adding the given shift and frequency ratio.
     * @param offset_from_master The shift to apply to the clock.
     */
    virtual void adjust(double offset_from_master) = 0;

    /**
     * Steps the clock to the given offset from the master clock. This is used when the clock is out of sync and needs
     * to be reset.
     * @param offset_from_master The offset from the master clock in seconds.
     */
    virtual void step(double offset_from_master) = 0;

    /**
     * @return The current frequency ratio of the clock.
     */
    [[nodiscard]] virtual double get_frequency_ratio() const = 0;
};

/**
 * A class that maintains a local PTP clock as close as possible to some grand master clock.
 * This particular implementation maintains a 'virtual' clock based on the monotonic system clock.
 */
class LocalSystemClock : public LocalClock {
  public:
    [[nodiscard]] Timestamp now() const override {
        return get_adjusted_time(system_monotonic_now());
    }

    void adjust(const double offset_from_master) override {
        TRACY_ZONE_SCOPED;
        last_sync_ = system_monotonic_now();
        shift_ += -offset_from_master;

        constexpr double max_ratio = 0.5;  // +/-
        const auto nominal_ratio = 0.001 * std::pow(-offset_from_master, 3) + 1.0;
        frequency_ratio_ = std::clamp(nominal_ratio, 1.0 - max_ratio, 1 + max_ratio);
    }

    void step(const double offset_from_master) override {
        TRACY_ZONE_SCOPED;
        last_sync_ = system_monotonic_now();
        shift_ = -offset_from_master;
        frequency_ratio_ = 1.0;
    }

    [[nodiscard]] double get_frequency_ratio() const override {
        return frequency_ratio_;
    }

  private:
    Timestamp last_sync_ = system_monotonic_now();
    double shift_ {};
    double frequency_ratio_ = 1.0;

    /**
     * @param system_time The local timestamp to convert to the timescale of the grand master clock.
     * @return The best estimate of the PTP time based on given system time.
     */
    [[nodiscard]] Timestamp get_adjusted_time(const Timestamp system_time) const {
        TRACY_ZONE_SCOPED;
        const auto elapsed = system_time.total_seconds_double() - last_sync_.total_seconds_double();
        auto result = last_sync_;
        result.add_seconds(elapsed * frequency_ratio_);
        result.add_seconds(shift_);
        return result;
    }

    /**
     * @return The current system time as a PTP timestamp. The timestamp is based on the high resolution clock and bears
     * no relation to wallclock time (UTC or TAI).
     */
    static Timestamp system_monotonic_now() {
        return Timestamp(HighResolutionClock::now());
    }
};

}
