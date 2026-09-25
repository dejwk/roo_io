#pragma once

#include "roo_io/core/input_iterator.h"

namespace roo_io {

/// Counts bytes consumed through a borrowed iterator without imposing a limit.
///
/// Status is cached at construction and after each transfer. Do not access the
/// source independently while this adapter is active. The adapter neither owns
/// nor closes the source. The count wraps modulo SIZE_MAX + 1; callers needing
/// a bounded transfer must check their budget or use LimitedInputIterator.
template <typename InputIterator>
class CountingInputIterator {
 public:
  /// Creates a zero-initialized counter borrowing @p input.
  explicit CountingInputIterator(InputIterator& input)
      : input_(input), status_(input.status()) {}

  CountingInputIterator(const CountingInputIterator&) = delete;
  CountingInputIterator& operator=(const CountingInputIterator&) = delete;
  CountingInputIterator(CountingInputIterator&&) = delete;
  CountingInputIterator& operator=(CountingInputIterator&&) = delete;

  /// Reads a byte and counts it only when the source reports success.
  byte read() {
    if (status_ != kOk) return byte{0};
    byte value = input_.read();
    status_ = input_.status();
    if (status_ != kOk) return byte{0};
    ++size_;
    return value;
  }

  /// Reads up to @p count bytes and counts the returned prefix, even on error.
  /// Zero progress with kOk becomes kReadError; zero-length reads do nothing.
  size_t read(byte* data, size_t count) {
    if (count == 0 || status_ != kOk) return 0;
    size_t read = input_.read(data, count);
    status_ = input_.status();
    size_ += read;
    if (read == 0 && status_ == kOk) status_ = kReadError;
    return read;
  }

  /// Delegates a skip to the source, counting @p count bytes on success.
  /// On failure, the count is unchanged: the source's void skip() cannot report
  /// partial progress. Zero-length skips do nothing.
  void skip(size_t count) {
    if (count == 0 || status_ != kOk) return;
    input_.skip(count);
    status_ = input_.status();
    if (status_ == kOk) size_ += count;
  }

  /// Returns consumed bytes, excluding any partial progress of a failed skip.
  size_t size() const { return size_; }

  /// Returns the cached source status or a sticky local failure.
  Status status() const { return status_; }

 private:
  InputIterator& input_;
  size_t size_ = 0;
  Status status_;
};

}  // namespace roo_io
