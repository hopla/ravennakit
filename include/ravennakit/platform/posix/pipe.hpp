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

#include "ravennakit/core/exception.hpp"

#include <unistd.h>

namespace rav::posix {

/**
 * Wrapper around the POSIX pipe() function.
 */
class pipe {
  public:
    /**
     * Constructs a pipe.
     * @throws rav::exception if pipe() fails.
     */
    pipe() {
        if (::pipe(fds_) == -1) {
            RAV_THROW_EXCEPTION("pipe() failed");
        }
    }

    ~pipe() {
        if (fds_[0] != 0) {
            ::close(fds_[0]);
        }
        if (fds_[1] != 0) {
            ::close(fds_[1]);
        }
    }

    /**
     * Writes data to the pipe.
     * @param data The data to write.
     * @param size The size of the data.
     * @throws rav::exception if write() fails.
     */
    void write(const void* data, const size_t size) const {
        if (::write(fds_[1], data, size) < 0) {
            RAV_THROW_EXCEPTION("write() failed");
        }
    }

    /**
     * @returns The read file descriptors.
     */
    [[nodiscard]] int read_fd() const {
        return fds_[0];
    }

    /**
     * @returns The write file descriptors.
     */
    [[nodiscard]] int write_fd() const {
        return fds_[1];
    }

  private:
    int fds_[2] = {0, 0};  // read and write file descriptors
};

}  // namespace rav::posix
