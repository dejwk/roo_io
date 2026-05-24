#pragma once

#include "roo_io/core/multipass_input_stream.h"

namespace roo_io {

/// Multipass input stream backed directly by a contiguous memory range.
template <typename PtrType>
class MemoryInputStream : public MultipassInputStream {
 public:
  /// Creates a detached memory stream with `kClosed` status.
  MemoryInputStream()
      : ptr_(nullptr), position_(0), size_(0), status_(kClosed) {}

  /// Opens a memory stream over `[begin, end)`.
  MemoryInputStream(PtrType begin, PtrType end)
      : ptr_(begin), position_(0), size_(end - begin), status_(kOk) {}

  /// Returns whether the stream is readable or positioned at end-of-stream.
  bool isOpen() const override {
    return status_ == kOk || status_ == kEndOfStream;
  }

  /// Reads up to `count` bytes from the current position.
  size_t read(byte* buf, size_t count) override {
    if (status_ != kOk) return 0;
    if (position_ >= size_) {
      status_ = kEndOfStream;
      return 0;
    }
    if (position_ + count > size_) {
      count = size_ - position_;
    }
    memcpy(buf, ptr_ + position_, count);
    position_ += count;
    return count;
  }

  /// Skips up to `count` bytes from the current position.
  void skip(uint64_t count) override {
    if (status_ != kOk) return;
    if (position_ > size_) {
      status_ = kEndOfStream;
      return;
    }
    position_ += count;
    if (position_ > size_) {
      position_ = size_;
      status_ = kEndOfStream;
    }
  }

  /// Returns the current byte offset from the start of the range.
  uint64_t position() const override { return position_; }

  /// Seeks to `position` within the backing range.
  void seek(uint64_t position) override {
    if (!isOpen()) return;
    position_ = position;
    status_ = kOk;
  }

  /// Returns the total byte length of the backing range.
  uint64_t size() override { return size_; }

  /// Closes the stream and resets the read cursor to the beginning.
  void close() override {
    if (!isOpen()) return;
    position_ = 0;
    status_ = kClosed;
  }

  /// Returns the current stream status.
  Status status() const override { return status_; }

 private:
  PtrType ptr_;
  size_t position_;
  size_t size_;
  Status status_;
};

}  // namespace roo_io