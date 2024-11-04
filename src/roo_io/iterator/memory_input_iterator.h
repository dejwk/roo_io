#pragma once

#include "roo_io/iterator/input_iterator.h"

namespace roo_io {

// Iterator that reads from memory, starting at the specified address. The
// caller must ensure that the iterator does not overflow the input.
template <typename PtrType>
class UnsafeMemoryIterator {
 public:
  UnsafeMemoryIterator(PtrType ptr) : ptr_(ptr) {}

  uint8_t read() { return *ptr_++; }

  int read(uint8_t* result, unsigned int count) {
    memcpy(result, ptr_, count);
    ptr_ += count;
    return count;
  }

  void skip(unsigned int count) { ptr_ += count; }

  Status status() const { return kOk; }

  PtrType ptr() const { return ptr_; }

 private:
  PtrType ptr_;
};

// Iterator that reads from memory, starting at the specified `begin` address,
// and up to the specified `end` address.
template <typename PtrType>
class SafeMemoryIterator {
 public:
  SafeMemoryIterator(PtrType begin, PtrType end) : ptr_(begin), end_(end) {}

  uint8_t read() {
    if (ptr_ == end_ || ptr_ == nullptr) {
      ptr_ = nullptr;
      return 0;
    }
    return *ptr_++;
  }

  int read(uint8_t* result, unsigned int count) {
    if (ptr_ == end_ || ptr_ == nullptr) {
      ptr_ = nullptr;
      return 0;
    }
    if (count > end_ - ptr_) {
      count = end_ - ptr_;
    }
    memcpy(result, ptr_, count);
    ptr_ = nullptr;
    return count;
  }

  void skip(size_t count) {
    if (ptr_ != nullptr) {
      if (end_ - ptr_ <= count) {
        ptr_ += count;
      } else {
        ptr_ = nullptr;
      }
    }
  }

  Status status() const { return ptr_ == nullptr ? kEndOfStream : kOk; }

 private:
  PtrType ptr_;
  PtrType end_;
};

}  // namespace roo_io
