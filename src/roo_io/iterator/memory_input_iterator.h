#pragma once

#include <cstring>

#include "roo_io/iterator/input_iterator.h"

namespace roo_io {

// An 'infinite' iterator that reads from memory, starting at the specified
// address. The caller must ensure that the iterator does not overflow the
// actual input.
//
// Memory footprint is a single pointer (4 bytes on 32-bit architectures).
//
// Implements the 'input iterator' template contract.
template <typename PtrType>
class UnsafeGenericMemoryIterator {
 public:
  UnsafeGenericMemoryIterator(PtrType ptr) : ptr_(ptr) {}

  byte read() { return *ptr_++; }

  unsigned int read(byte* result, unsigned int count) {
    memcpy(result, ptr_, count);
    ptr_ += count;
    return count;
  }

  void skip(unsigned int count) { ptr_ += count; }

  Status status() const { return kOk; }

  PtrType ptr() const { return ptr_; }

 protected:
  PtrType ptr_;
};

using UnsafeMemoryIterator = UnsafeGenericMemoryIterator<const byte*>;

// Iterator that reads from memory, starting at the specified `begin` address,
// and up to the specified `end` address.
//
// Memory footprint is two pointers (8 bytes on 32-bit architectures).
//
// Implements the 'input iterator' template contract.
template <typename PtrType>
class SafeGenericMemoryIterator {
 public:
  SafeGenericMemoryIterator(PtrType begin, PtrType end)
      : ptr_(begin), end_(end) {}

  byte read() {
    if (ptr_ == end_ || ptr_ == nullptr) {
      ptr_ = nullptr;
      return 0;
    }
    return *ptr_++;
  }

  unsigned int read(byte* result, unsigned int count) {
    if (ptr_ == end_ || ptr_ == nullptr) {
      ptr_ = nullptr;
      return 0;
    }
    if (count > end_ - ptr_) {
      count = end_ - ptr_;
    }
    memcpy(result, ptr_, count);
    ptr_ += count;
    return count;
  }

  void skip(unsigned int count) {
    if (ptr_ != nullptr) {
      if (count <= end_ - ptr_) {
        ptr_ += count;
      } else {
        ptr_ = nullptr;
      }
    }
  }

  Status status() const { return ptr_ == nullptr ? kEndOfStream : kOk; }

  PtrType ptr() const { return ptr_; }

 protected:
  PtrType ptr_;
  PtrType end_;
};

using MemoryIterator = SafeGenericMemoryIterator<const byte*>;

// Iterator that reads from memory, starting at the specified `begin` address,
// and up to the specified `end` address.
//
// Memory footprint is three pointers (12 bytes on 32-bit architectures).
//
// Implements the 'multipass input iterator' template contract.
template <typename PtrType>
class MultipassGenericMemoryIterator
    : public SafeGenericMemoryIterator<PtrType> {
 public:
  MultipassGenericMemoryIterator(PtrType begin, PtrType end)
      : SafeGenericMemoryIterator<PtrType>(begin, end), begin_(begin) {}

  uint64_t size() const {
    return SafeGenericMemoryIterator<PtrType>::end_ - begin_;
  }

  uint64_t position() const {
    return SafeGenericMemoryIterator<PtrType>::ptr_ == nullptr
               ? size()
               : SafeGenericMemoryIterator<PtrType>::ptr_ - begin_;
  }

  void rewind() { SafeGenericMemoryIterator<PtrType>::ptr_ = begin_; }

  void seek(uint64_t position) {
    SafeGenericMemoryIterator<PtrType>::ptr_ =
        (position > size()) ? nullptr : begin_ + position;
  }

 private:
  PtrType begin_;
};

using MultipassMemoryIterator = MultipassGenericMemoryIterator<const byte*>;

}  // namespace roo_io
