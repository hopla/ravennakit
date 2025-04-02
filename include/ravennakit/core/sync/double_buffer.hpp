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

#include <atomic>
#include <memory>
#include <optional>

namespace rav {

/**
 * A single-producer, single-consumer double buffer. The producer and consumer can be on different threads. Access to
 * the buffer is wait-free. Borrowed from https://stackoverflow.com/a/23713793/2576876.
 * NOTE: The original implementation triggered TSAN warnings. The atomic operations have been made more strict with
 * sequential ordering, but this hurts the performance of this container. At some point the atomic operations should be
 * loosened where possible.
 */
template<typename T>
class DoubleBuffer {
  public:
    DoubleBuffer() = default;

    DoubleBuffer(const DoubleBuffer&) = delete;
    DoubleBuffer& operator=(const DoubleBuffer&) = delete;

    DoubleBuffer(DoubleBuffer&&) = delete;
    DoubleBuffer& operator=(DoubleBuffer&&) = delete;

    /**
     * Updates the value.
     * @param value The new value.
     */
    void update(T value) {
        T* buffer = start_writing();
        *buffer = value;
        end_writing();
    }

    /**
     * @returns A new value. If there's nothing to read, returns std::nullopt. A new value will be available once.
     */
    std::optional<T> get() {
        T* buffer = start_reading();
        if (buffer == nullptr) {
            return std::nullopt;  // If there's nothing to read, return empty optional
        }
        T value = *buffer;
        end_reading();
        return value;
    }

  private:
    // Never returns nullptr
    T* start_writing() {
        // Increment active users; once we do this, no one can swap the active cell on us until we're done
        const auto state = state_.fetch_add(0x2);
        return &storage_[state & 1];
    }

    void end_writing() {
        // We want to swap the active cell, but only if we were the last ones concurrently accessing the data (otherwise
        // the consumer will do it for us when *it's* done accessing the data)

        auto state = state_.load();

        // Set the "full" flag for the buffer we just wrote to
        const uint32_t flag = (8u << (state & 1)) ^ (state & (8u << (state & 1)));

        // Decrement the user count and set the flag atomically
        state = state_.fetch_add(flag - 0x2) + flag - 0x2;

        if ((state & 0x6) == 0) {
            // The consumer wasn't in the middle of a read, we should swap (unless the consumer has since started a read
            // or already swapped or read a value and is about to swap). If we swap, we also want to clear the full flag
            // on what will become the active cell, otherwise the consumer could eventually read two values out of order
            // (it reads a new value, then swaps and reads the old value while the producer is idle).
            state_.compare_exchange_strong(state, (state ^ 0x1) & ~(0x10 >> (state & 1)));
        }
    }

    // Returns nullptr if there appears to be no more data to read yet
    T* start_reading() {
        read_state_ = state_.load();

        // Check if there's a value available in the inactive buffer
        if ((read_state_ & (0x10 >> (read_state_ & 1))) == 0) {
            // Nothing to read here!
            return nullptr;
        }

        // At this point, there is guaranteed to be something to read, because the full flag is never turned off by the
        // producer thread once it's on; the only thing that could happen is that the active cell changes, but that can
        // only happen after the producer wrote a value into it, in which case there's still a value to read, just in a
        // different cell.
        read_state_ = state_.fetch_add(0x2) + 0x2;

        // Now that we've incremented the user count, nobody can swap until we decrement it
        return &storage_[(read_state_ & 1) ^ 1];
    }

    void end_reading() {
        if ((read_state_ & (0x10 >> (read_state_ & 1))) == 0) {
            // There was nothing to read
            return;
        }

        // Alright, at this point the active cell cannot change on us, but the active cell's flag could change and the
        // user count could change. We want to release our user count and remove the flag on the value we read.
        auto state = state_.load();
        const uint32_t sub = (0x10 >> (state & 1)) | 0x2;
        state = state_.fetch_sub(sub) - sub;

        if ((state & 0x6) == 0 && (state & (0x8u << (state & 1))) == 1) {
            // We were the last ones accessing the data when we released our cell. That means we should swap, but only
            // if the producer isn't in the middle of producing something, and hasn't already swapped, and hasn't
            // already set the flag we just reset (which would mean they swapped an even number of times). Note that we
            // don't bother swapping if there's nothing to read in the other cell.
            state_.compare_exchange_strong(state, state ^ 0x1);
        }
    }

    T storage_[2];

    // The bottom (lowest) bit will be the active cell (the one for writing). The active cell can only be switched if
    // there's at most one concurrent user. The next two bits of state will be the number of concurrent users. The
    // fourth bit indicates if there's a value available for reading in m_buf[0], and the fifth bit has the same meaning
    // but for m_buf[1].
    std::atomic<uint32_t> state_ {};

    uint32_t read_state_ {};
};

}  // namespace rav
