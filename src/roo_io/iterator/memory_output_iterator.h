#pragma once

#include <cstdint>
#include <cstring>

#include "roo_io/iterator/output_iterator.h"

namespace roo_io {

// Iterator that writes to memory, starting at the specified address. The caller
// must ensure that the iterator does not overflow the output.
class UnsafeMemoryOutputIterator {
 public:
  UnsafeMemoryOutputIterator(uint8_t* ptr) : ptr_(ptr) {}

  void write(uint8_t v) { *ptr_++ = v; }

  unsigned int write(const uint8_t* buf, unsigned int count) {
    memcpy(ptr_, buf, count);
    ptr_ += count;
    return count;
  }

  Status status() const { return kOk; }

  uint8_t* ptr() const { return ptr_; }

 private:
  uint8_t* ptr_;
};

// Iterator that writes to memory, starting at the specified address. Won't
// write past the end address (returning kNoSpaceLeftOnDevice if such write is
// attempted). must ensure that the iterator does not overflow the output.
class MemoryOutputIterator {
 public:
  MemoryOutputIterator(uint8_t* ptr, const uint8_t* end)
      : ptr_(ptr), end_(end) {}

  // Writes `v`, or sets status to 'kNoSpaceLeftOnDevice' if there is no more
  // space.
  void write(uint8_t v) {
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
  unsigned int write(const uint8_t* buf, unsigned int count) {
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

  uint8_t* ptr() const { return ptr_; }

 private:
  uint8_t* ptr_;
  const uint8_t* end_;
};

}  // namespace roo_io
