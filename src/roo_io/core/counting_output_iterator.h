#pragma once

#include <cstddef>

#include "roo_io/core/output_iterator.h"

namespace roo_io {

/// Counts bytes accepted by a borrowed iterator without imposing a limit.
///
/// Status is cached at construction and after each operation. Do not access the
/// sink independently while this adapter is active. The adapter neither owns
/// nor closes the sink, and destruction does not flush it. The count wraps
/// modulo SIZE_MAX + 1; callers needing bounded writes must check their budget
/// or use LimitedOutputIterator. Use CountingOutputSink for sizing without I/O.
template <typename OutputIterator>
class CountingOutputIterator {
 public:
  /// Creates a zero-initialized counter borrowing @p output.
  explicit CountingOutputIterator(OutputIterator& output)
      : output_(output), status_(output.status()) {}

  CountingOutputIterator(const CountingOutputIterator&) = delete;
  CountingOutputIterator& operator=(const CountingOutputIterator&) = delete;
  CountingOutputIterator(CountingOutputIterator&&) = delete;
  CountingOutputIterator& operator=(CountingOutputIterator&&) = delete;

  /// Writes a byte and counts it only when the sink reports success.
  void write(byte value) {
    if (status_ != kOk) return;
    output_.write(value);
    status_ = output_.status();
    if (status_ == kOk) ++size_;
  }

  /// Forwards one bulk write and counts the returned prefix, even on error.
  /// Zero progress with kOk becomes kWriteError; zero-length writes do nothing.
  size_t write(const byte* data, size_t count) {
    if (count == 0 || status_ != kOk) return 0;
    size_t written = output_.write(data, count);
    status_ = output_.status();
    size_ += written;
    if (written == 0 && status_ == kOk) status_ = kWriteError;
    return written;
  }

  /// Flushes the borrowed sink and caches its status, unless already failed.
  void flush() {
    if (status_ != kOk) return;
    output_.flush();
    status_ = output_.status();
  }

  /// Returns bytes successfully accepted by the sink.
  size_t size() const { return size_; }

  /// Returns the cached sink status or a sticky local failure.
  Status status() const { return status_; }

 private:
  OutputIterator& output_;
  size_t size_ = 0;
  Status status_;
};

}  // namespace roo_io
