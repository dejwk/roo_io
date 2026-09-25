#pragma once

#include <cstddef>

#include "roo_io/core/output_iterator.h"

namespace roo_io {

/// Borrowed output window that accepts at most a fixed number of bytes.
///
/// The sink must not be used independently while this adapter is active.
/// Status is sampled at construction and after each sink operation. The adapter
/// does not own, close, copy, or move the sink, and destruction does not flush
/// it.
template <typename OutputIterator>
class LimitedOutputIterator {
 public:
  /// Creates a window accepting at most @p limit bytes through @p output.
  LimitedOutputIterator(OutputIterator& output, size_t limit)
      : output_(output), remaining_(limit), status_(output.status()) {}

  LimitedOutputIterator(const LimitedOutputIterator&) = delete;
  LimitedOutputIterator& operator=(const LimitedOutputIterator&) = delete;
  LimitedOutputIterator(LimitedOutputIterator&&) = delete;
  LimitedOutputIterator& operator=(LimitedOutputIterator&&) = delete;

  /// Writes one byte, or reports kNoSpaceLeftOnDevice at the window boundary.
  void write(byte value) {
    if (status_ != kOk) return;
    if (remaining_ == 0) {
      status_ = kNoSpaceLeftOnDevice;
      return;
    }
    output_.write(value);
    status_ = output_.status();
    if (status_ == kOk) --remaining_;
  }

  /// Writes up to @p count bytes in one underlying transfer.
  ///
  /// Requests exceeding the window fail before writing any bytes. Other errors
  /// can leave a written prefix, whose length is returned. Zero progress with
  /// kOk becomes kWriteError. A zero-length write has no effect.
  /// The buffer must satisfy the underlying sink's requirements.
  size_t write(const byte* data, size_t count) {
    if (count == 0 || status_ != kOk) return 0;
    if (count > remaining_) {
      status_ = kNoSpaceLeftOnDevice;
      return 0;
    }
    size_t written = output_.write(data, count);
    status_ = output_.status();
    if (written > count) {
      status_ = kWriteError;
      return 0;
    }
    remaining_ -= written;
    if (written == 0 && status_ == kOk) status_ = kWriteError;
    return written;
  }

  /// Flushes the borrowed sink and caches its status, unless already failed.
  void flush() {
    if (status_ != kOk) return;
    output_.flush();
    status_ = output_.status();
  }

  /// Returns the number of bytes still permitted by the window.
  size_t remaining() const { return remaining_; }

  /// Returns the cached sink status or a sticky local failure.
  Status status() const { return status_; }

 private:
  OutputIterator& output_;
  size_t remaining_;
  Status status_;
};

}  // namespace roo_io
