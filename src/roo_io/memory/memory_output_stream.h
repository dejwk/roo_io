#pragma once

#include "roo_io/core/output_stream.h"

namespace roo_io {

/// Output stream backed directly by a caller-provided memory range.
template <typename PtrType>
class MemoryOutputStream : public OutputStream {
 public:
  /// Creates a detached memory stream with `kClosed` status.
  MemoryOutputStream() : ptr_(nullptr), end_(nullptr), status_(kClosed) {}

  /// Opens a memory stream that writes into `[begin, end)`.
  MemoryOutputStream(PtrType begin, PtrType end)
      : ptr_(begin), end_(end), status_(kOk) {}

  /// Writes up to `count` bytes into the remaining memory range.
  size_t write(const byte* buf, size_t count) override {
    if (status_ != kOk) return 0;
    const size_t available = static_cast<size_t>(end_ - ptr_);
    if (count > available) {
      count = available;
      status_ = kNoSpaceLeftOnDevice;
    }
    memcpy(ptr_, buf, count);
    ptr_ += count;
    return count;
  }

  /// Closes the stream when it is still healthy.
  void close() override {
    if (status_ == kOk) {
      status_ = kClosed;
    }
  }

  /// Returns the current stream status.
  Status status() const override { return status_; }

  /// Returns the next write position inside the backing range.
  byte* ptr() const { return ptr_; }

 private:
  byte* ptr_;
  const byte* end_;
  Status status_;
};

}  // namespace roo_io