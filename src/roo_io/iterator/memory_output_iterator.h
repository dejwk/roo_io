#pragma once

#include <cstdint>
#include <cstring>

#include "roo_io/iterator/output_iterator.h"

namespace roo_io {

// Iterator that writes to memory, starting at the specified address. The caller
// must ensure that the iterator does not overflow the output.
class UnsafeMemoryOutputIterator {
 public:
  UnsafeMemoryOutputIterator(byte* ptr) : ptr_(ptr) {}

  void write(byte v) { *ptr_++ = v; }

  size_t write(const byte* buf, size_t count) {
    memcpy(ptr_, buf, count);
    ptr_ += count;
    return count;
  }

  Status status() const { return kOk; }

  byte* ptr() const { return ptr_; }

  void flush() {}

 private:
  byte* ptr_;
};

// Iterator that writes to memory, starting at the specified address. Won't
// write past the end address (returning kNoSpaceLeftOnDevice if such write is
// attempted). must ensure that the iterator does not overflow the output.
class MemoryOutputIterator {
 public:
  MemoryOutputIterator(byte* ptr, const byte* end) : ptr_(ptr), end_(end) {}

  // Writes `v`, or sets status to 'kNoSpaceLeftOnDevice' if there is no more
  // space.
  void write(byte v) {
    if (ptr_ == nullptr) {
      return;
    }
    if (ptr_ == end_) {
      ptr_ = nullptr;
      return;
    }
    *ptr_++ = v;
  }

  // Writes the `count` bytes, or writes as many bytes as possible and sets
  // status to 'kNoSpaceLeftOnDevice' if there is not enough space to write
  // `count` butes.
  size_t write(const byte* buf, size_t count) {
    if (ptr_ == nullptr) return 0;
    if (count > end_ - ptr_) {
      count = end_ - ptr_;
      memcpy(ptr_, buf, count);
      ptr_ = nullptr;
      return count;
    }
    memcpy(ptr_, buf, count);
    ptr_ += count;
    return count;
  }

  Status status() const { return ptr_ == nullptr ? kNoSpaceLeftOnDevice : kOk; }

  const byte* ptr() const { return ptr_ != nullptr ? ptr_ : end_; }

  void flush() {}

 private:
  byte* ptr_;
  const byte* end_;
};

}  // namespace roo_io
