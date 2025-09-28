#pragma once

#include <cstring>
#include <memory>

#include "roo_io/base/byte.h"

namespace roo_io {

class RingBuffer {
 public:
  RingBuffer(size_t capacity)
      : buf_(new byte[capacity]), capacity_(capacity), head_(0), used_(0) {}

  size_t capacity() const { return capacity_; }
  size_t used() const { return used_; }
  size_t free() const { return capacity_ - used_; }

  bool full() const { return used_ == capacity_; }
  bool empty() const { return used_ == 0; }

  void clear() {
    head_ = 0;
    used_ = 0;
  }

  // Writes up to 'len' bytes, but no more than the current value of free(), to
  // the ringbuffer. Returns the count of bytes written.
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

  // Writes a single byte to the ringbuffer. Returns true if successful, or
  // false if the buffer is full.
  bool write(byte b) {
    if (used_ == capacity_) return false;
    buf_[write_pos()] = b;
    used_++;
    return true;
  }

  // Reads a single byte from the ringbuffer. Returns true if successful, or
  // false if the buffer is empty.
  bool read(byte* b) {
    if (used_ == 0) return false;
    *b = buf_[head_];
    head_++;
    if (head_ == capacity_) head_ = 0;
    used_--;
    return true;
  }

  // Reads up to 'len' bytes, but no more than the current value of used(), from
  // the ringbuffer. Returns the count of bytes read.
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
    if (pos > capacity_) pos -= capacity_;
    return pos;
  }

  std::unique_ptr<byte[]> buf_;
  size_t capacity_;
  size_t head_;
  size_t used_;
};

}  // namespace roo_io