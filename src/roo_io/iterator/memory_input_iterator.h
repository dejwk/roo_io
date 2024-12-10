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

  size_t read(byte* result, size_t count) {
    memcpy(result, ptr_, count);
    ptr_ += count;
    return count;
  }

  void skip(size_t count) { ptr_ += count; }

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
      return byte{0};
    }
    return *ptr_++;
  }

  size_t read(byte* result, size_t count) {
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

  void skip(size_t count) {
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

 private:
  PtrType ptr_;
  PtrType end_;
};

using MemoryIterator = SafeGenericMemoryIterator<const byte*>;

// Iterator that reads from memory, starting at the specified `begin` address,
// and up to the specified `end` address.
//
// Memory footprint is three pointers and a boolean.
//
// Implements the 'multipass input iterator' template contract.
template <typename PtrType>
class MultipassGenericMemoryIterator {
 public:
  MultipassGenericMemoryIterator(PtrType begin, PtrType end)
      : ptr_(begin), position_(0), size_(end - begin), eos_(false) {}

  byte read() {
    if (position_ >= size_) {
      eos_ = true;
      return byte{0};
    }
    return ptr_[position_++];
  }

  size_t read(byte* result, size_t count) {
    if (position_ >= size_) {
      eos_ = true;
      return 0;
    }
    if (count > size_ - position_) {
      count = size_ - position_;
    }
    memcpy(result, ptr_ + position_, count);
    position_ += count;
    return count;
  }

  void skip(size_t count) {
    if (position_ + count <= size_) {
      position_ += count;
    } else {
      position_ = size_;
      eos_ = true;
      return;
    }
  }

  Status status() const { return eos_ ? kEndOfStream : kOk; }

  PtrType ptr() const { return ptr_ + position_; }

  uint64_t size() const { return size_; }

  uint64_t position() const { return position_; }

  void rewind() {
    position_ = 0;
    eos_ = false;
  }

  void seek(uint64_t position) {
    position_ = position;
    eos_ = false;
  }

 private:
  PtrType ptr_;
  size_t position_;
  size_t size_;
  bool eos_;
};

using MultipassMemoryIterator = MultipassGenericMemoryIterator<const byte*>;

}  // namespace roo_io
