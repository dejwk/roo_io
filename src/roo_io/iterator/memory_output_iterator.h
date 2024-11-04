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

  int write(const uint8_t* buf, unsigned int len) {
    memcpy(ptr_, buf, len);
    ptr_ += len;
    return len;
  }

  Status status() const { return kOk; }

  uint8_t* ptr() const { return ptr_; }

 private:
  uint8_t* ptr_;
};

class SafeMemoryOutputIterator {
 public:
  SafeMemoryOutputIterator(uint8_t* ptr, const uint8_t* end)
      : ptr_(ptr), end_(end) {}

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

  int write(const uint8_t* buf, unsigned int len) {
    if (ptr_ == nullptr) return -1;
    if (len > end_ - ptr_) {
      len = end_ - ptr_;
      memcpy(ptr_, buf, len);
      ptr_ = nullptr;
      return len;
    }
    memcpy(ptr_, buf, len);
    ptr_ += len;
    return len;
  }

  Status status() const { return ptr_ == nullptr ? kNoSpaceLeftOnDevice : kOk; }

  uint8_t* ptr() const { return ptr_; }

 private:
  uint8_t* ptr_;
  const uint8_t* end_;
};

}  // namespace roo_io
