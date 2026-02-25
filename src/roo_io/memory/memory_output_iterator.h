#pragma once

#include <cstdint>
#include <cstring>

#include "roo_io/core/output_iterator.h"

namespace roo_io {

/// Infinite output iterator writing to memory from given address.
///
/// Writer controls bounds externally.
class UnsafeMemoryOutputIterator {
 public:
  /// Creates iterator starting at `ptr`.
  UnsafeMemoryOutputIterator(byte* ptr) : ptr_(ptr) {}

  /// Writes one byte and advances output pointer.
  ///
  /// Does not modify status (`status()` is always `kOk`).
  void write(byte v) { *ptr_++ = v; }

  /// Writes `count` bytes and advances output pointer.
  ///
  /// Does not modify status (`status()` is always `kOk`).
  ///
  /// @return Number of bytes written (`count`).
  size_t write(const byte* buf, size_t count) {
    memcpy(ptr_, buf, count);
    ptr_ += count;
    return count;
  }

  /// Returns current status (`kOk`).
  ///
  /// @return `kOk`.
  Status status() const { return kOk; }

  /// Returns current output pointer.
  ///
  /// @return Current pointer.
  byte* ptr() const { return ptr_; }

  /// Flushes output (no-op).
  ///
  /// Does not modify status.
  void flush() {}

 private:
  byte* ptr_;
};

/// Bounded output iterator writing from given address up to `end`.
///
/// Writing past end is rejected and status becomes `kNoSpaceLeftOnDevice`.
class MemoryOutputIterator {
 public:
  /// Creates bounded iterator over [`ptr`, `end`).
  MemoryOutputIterator(byte* ptr, const byte* end) : ptr_(ptr), end_(end) {}

  /// Creates bounded iterator over fixed array.
  template <size_t N>
  MemoryOutputIterator(byte buf[N]) : ptr_(buf), end_(&buf[N]) {}

  /// Writes one byte.
  ///
  /// If no space remains, write is ignored and status becomes
  /// `kNoSpaceLeftOnDevice`.
  void write(byte v) {
    if (end_ == nullptr) {
      return;
    }
    if (ptr_ == end_) {
      end_ = nullptr;
      return;
    }
    *ptr_++ = v;
  }

  /// Writes up to `count` bytes.
  ///
  /// If available space is smaller than `count`, writes available prefix,
  /// updates status to `kNoSpaceLeftOnDevice`, and returns bytes written.
  ///
  /// @return Number of bytes written.
  size_t write(const byte* buf, size_t count) {
    if (end_ == nullptr) return 0;
    if (count > (size_t)(end_ - ptr_)) {
      count = end_ - ptr_;
      end_ = nullptr;
    }
    memcpy(ptr_, buf, count);
    ptr_ += count;
    return count;
  }

  /// Returns current status.
  ///
  /// @return `kOk` when space remains; `kNoSpaceLeftOnDevice` after overflow.
  Status status() const { return end_ == nullptr ? kNoSpaceLeftOnDevice : kOk; }

  /// Returns current output pointer.
  ///
  /// @return Current pointer.
  const byte* ptr() const { return ptr_; }

  /// Flushes output (no-op).
  ///
  /// Does not modify status.
  void flush() {}

 private:
  byte* ptr_;
  const byte* end_;
};

template <typename Collection>
class BackInsertingIterator {
 public:
  /// Creates iterator appending to collection `c`.
  BackInsertingIterator(Collection& c) : itr_(std::back_inserter(c)) {}

  /// Appends one byte to target collection.
  ///
  /// Does not modify status (`status()` is always `kOk`).
  void write(byte b) { *itr_++ = (typename Collection::value_type)b; }

  /// Appends `count` bytes to target collection.
  ///
  /// Does not modify status (`status()` is always `kOk`).
  ///
  /// @return Number of bytes appended (`count`).
  size_t write(const byte* buf, size_t count) {
    for (size_t i = 0; i < count; ++i) write(buf[i]);
    return count;
  }

  /// Flushes output (no-op).
  ///
  /// Does not modify status.
  void flush() {}

  /// Returns current status (`kOk`).
  ///
  /// @return `kOk`.
  Status status() const { return kOk; }

 private:
  std::back_insert_iterator<Collection> itr_;
};

}  // namespace roo_io
