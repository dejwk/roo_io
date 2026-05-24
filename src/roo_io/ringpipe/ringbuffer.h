#pragma once

#include <cstring>
#include <memory>

#include "roo_io/base/byte.h"

namespace roo_io {

/// Fixed-size circular byte buffer with non-blocking read and write helpers.
class RingBuffer {
 public:
  /// Allocates a ring buffer with the specified byte capacity.
  RingBuffer(size_t capacity)
      : buf_(new byte[capacity]), capacity_(capacity), head_(0), used_(0) {}

  /// Returns the total number of bytes the buffer can hold.
  size_t capacity() const { return capacity_; }
  /// Returns the number of bytes currently stored in the buffer.
  size_t used() const { return used_; }
  /// Returns the number of free bytes currently available for writing.
  size_t free() const { return capacity_ - used_; }

  /// Returns whether no additional bytes can be written.
  bool full() const { return used_ == capacity_; }
  /// Returns whether the buffer currently holds no data.
  bool empty() const { return used_ == 0; }

  /// Removes all buffered bytes.
  void clear() {
    head_ = 0;
    used_ = 0;
  }

  /// Writes up to `len` bytes, capped by the current free capacity.
  size_t write(const byte* data, size_t len) {
    size_t maxlen = free();
    if (len > maxlen) len = maxlen;
    if (len == 0) return 0;
    size_t pos = write_pos();
    size_t split = capacity_ - pos;
    if (len < split) {
      // Fits; just write and return.
      memcpy(&buf_[pos], data, len);
    } else {
      // Break into two pieces.
      memcpy(&buf_[pos], data, split);
      memcpy(&buf_[0], data + split, len - split);
    }
    used_ += len;
    return len;
  }

  /// Writes one byte and returns whether it fit in the buffer.
  bool write(byte b) {
    if (used_ == capacity_) return false;
    buf_[write_pos()] = b;
    used_++;
    return true;
  }

  /// Reads one byte into `b` and returns whether data was available.
  bool read(byte* b) {
    if (used_ == 0) return false;
    *b = buf_[head_];
    head_++;
    if (head_ == capacity_) head_ = 0;
    used_--;
    return true;
  }

  /// Reads up to `len` bytes, capped by the currently buffered byte count.
  size_t read(byte* data, size_t len) {
    if (len > used_) len = used_;
    if (len == 0) return 0;
    size_t split = capacity_ - head_;
    if (len < split) {
      memcpy(data, &buf_[head_], len);
      head_ += len;
    } else {
      memcpy(data, &buf_[head_], split);
      memcpy(data + split, &buf_[0], len - split);
      head_ = len - split;
    }
    used_ -= len;
    return len;
  }

 private:
  size_t write_pos() const {
    size_t pos = head_ + used_;
    if (pos >= capacity_) pos -= capacity_;
    return pos;
  }

  std::unique_ptr<byte[]> buf_;
  size_t capacity_;
  size_t head_;
  size_t used_;
};

}  // namespace roo_io