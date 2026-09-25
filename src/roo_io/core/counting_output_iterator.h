#pragma once

#include <limits>

#include "roo_io/core/output_iterator.h"

namespace roo_io {

/// Output iterator that counts accepted bytes without storing them.
///
/// Writes exceeding `limit` fail atomically with `kNoSpaceLeftOnDevice`; bulk
/// writes never dereference their source buffer.
class CountingOutputIterator {
 public:
  /// Creates a counting sink with an optional maximum accepted byte count.
  explicit CountingOutputIterator(size_t limit = SIZE_MAX)
      : size_(0), limit_(limit), status_(kOk) {}

  /// Counts one byte when capacity remains.
  void write(byte /*value*/) {
    if (status_ != kOk) return;
    if (size_ == limit_) {
      status_ = kNoSpaceLeftOnDevice;
      return;
    }
    ++size_;
  }

  /// Counts a complete bulk write when capacity remains.
  size_t write(const byte* /*data*/, size_t count) {
    if (count == 0) return 0;
    if (status_ != kOk) return 0;
    if (count > limit_ - size_) {
      status_ = kNoSpaceLeftOnDevice;
      return 0;
    }
    size_ += count;
    return count;
  }

  /// Flushes this non-buffering iterator (no-op).
  void flush() {}

  /// Returns the number of accepted bytes.
  size_t size() const { return size_; }

  /// Returns the sink status.
  Status status() const { return status_; }

 private:
  size_t size_;
  size_t limit_;
  Status status_;
};

}  // namespace roo_io
