#pragma once

#include "roo_io/core/input_iterator.h"

namespace roo_io {

/// Borrowed input-iterator window that exposes at most `limit` source bytes.
///
/// The source must not be read directly while this adapter is active. The
/// adapter does not own, close, copy, or move the source iterator. Status is
/// sampled at construction and after each source read; do not mutate the source
/// independently while this adapter is active.
template <typename InputIterator>
class LimitedInputIterator {
 public:
  /// Creates a byte-bounded window over `input` without reading it.
  LimitedInputIterator(InputIterator& input, size_t limit)
      : input_(input), remaining_(limit), status_(input.status()) {}

  LimitedInputIterator(const LimitedInputIterator&) = delete;
  LimitedInputIterator& operator=(const LimitedInputIterator&) = delete;
  LimitedInputIterator(LimitedInputIterator&&) = delete;
  LimitedInputIterator& operator=(LimitedInputIterator&&) = delete;

  /// Reads one byte when the window and source both remain healthy.
  byte read() {
    if (status_ != kOk) return byte{0};
    if (remaining_ == 0) {
      status_ = kEndOfStream;
      return byte{0};
    }
    byte result = input_.read();
    status_ = input_.status();
    if (status_ != kOk) return byte{0};
    --remaining_;
    return result;
  }

  /// Reads up to `count` bytes without crossing the window boundary.
  size_t read(byte* result, size_t count) {
    if (count == 0) return 0;
    if (status_ != kOk) return 0;
    if (remaining_ == 0) {
      status_ = kEndOfStream;
      return 0;
    }
    if (count > remaining_) count = remaining_;
    size_t read = input_.read(result, count);
    status_ = input_.status();
    remaining_ -= read;
    if (read == 0 && status_ == kOk) status_ = kReadError;
    return read;
  }

  /// Skips up to `count` bytes without crossing the window boundary.
  void skip(size_t count) {
    byte scratch[64];
    while (count > 0 && status_ == kOk) {
      size_t chunk = count < sizeof(scratch) ? count : sizeof(scratch);
      size_t read = this->read(scratch, chunk);
      if (read == 0) return;
      count -= read;
    }
  }

  /// Returns the remaining bytes available through this window.
  size_t remaining() const { return remaining_; }

  /// Returns the cached source status or a sticky local failure.
  Status status() const { return status_; }

 private:
  InputIterator& input_;
  size_t remaining_;
  Status status_;
};

}  // namespace roo_io
