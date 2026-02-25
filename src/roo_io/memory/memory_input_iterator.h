#pragma once

#include <cstring>

#include "roo_io/core/input_iterator.h"

namespace roo_io {

/// Infinite iterator reading from memory starting at provided address.
///
/// Reader controls bounds externally.
///
/// Memory footprint: one pointer (4 bytes on 32-bit targets).
///
/// Implements input iterator contract.
template <typename PtrTypeT>
class UnsafeGenericMemoryIterator {
 public:
  using PtrType = PtrTypeT;

  /// Creates iterator starting at `ptr`.
  UnsafeGenericMemoryIterator(PtrType ptr) : ptr_(ptr) {}

  /// Returns byte at current pointer and advances by one.
  ///
  /// Does not modify status (`status()` is always `kOk`).
  byte read() { return *ptr_++; }

  /// Copies `count` bytes from current pointer, then advances by `count`.
  ///
  /// Does not modify status (`status()` is always `kOk`).
  size_t read(byte* result, size_t count) {
    memcpy(result, ptr_, count);
    ptr_ += count;
    return count;
  }

  /// Advances pointer by `count` bytes.
  ///
  /// Does not modify status (`status()` is always `kOk`).
  void skip(size_t count) { ptr_ += count; }

  /// Returns `kOk` unconditionally.
  Status status() const { return kOk; }

  /// Returns current pointer.
  PtrType ptr() const { return ptr_; }

 protected:
  PtrTypeT ptr_;
};

using UnsafeMemoryIterator = UnsafeGenericMemoryIterator<const byte*>;

/// Bounded iterator reading from [`begin`, `end`).
///
/// Reaching `end` transitions status to `kEndOfStream`.
///
/// Memory footprint: two pointers (8 bytes on 32-bit targets).
///
/// Implements input iterator contract.
template <typename PtrTypeT>
class SafeGenericMemoryIterator {
 public:
  using PtrType = PtrTypeT;

  /// Creates bounded iterator over [`begin`, `end`).
  SafeGenericMemoryIterator(PtrType begin, PtrType end)
      : ptr_(begin), end_(end) {}

  /// Reads one byte when available.
  ///
  /// If already exhausted, returns zero byte and keeps EOS state.
  /// If this call detects exhaustion, marks EOS by setting internal end marker
  /// to null.
  byte read() {
    if (ptr_ == end_ || end_ == nullptr) {
      end_ = nullptr;
      return byte{0};
    }
    return *ptr_++;
  }

  /// Reads up to `count` bytes when available.
  ///
  /// Returns zero when exhausted and keeps EOS state.
  /// If this call detects exhaustion before reading, marks EOS.
  /// Reading exactly to the original end keeps status `kOk` until a later call
  /// detects exhaustion.
  size_t read(byte* result, size_t count) {
    if (ptr_ == end_ || end_ == nullptr) {
      end_ = nullptr;
      return 0;
    }
    const size_t available = static_cast<size_t>(end_ - ptr_);
    if (count > available) {
      count = available;
    }
    memcpy(result, ptr_, count);
    ptr_ += count;
    return count;
  }

  /// Skips up to `count` bytes.
  ///
  /// Skipping past available range sets EOS.
  /// Skipping exactly to end keeps status `kOk` until a later operation
  /// detects exhaustion.
  void skip(size_t count) {
    if (end_ != nullptr) {
      const size_t available = static_cast<size_t>(end_ - ptr_);
      if (count <= available) {
        ptr_ += count;
      } else {
        ptr_ = end_;
        end_ = nullptr;
      }
    }
  }

  /// Returns `kEndOfStream` after EOS marker is set, otherwise `kOk`.
  Status status() const { return end_ == nullptr ? kEndOfStream : kOk; }

  /// Returns current pointer.
  PtrType ptr() const { return ptr_; }
  // PtrType end() const { return end_; }

 private:
  PtrType ptr_;
  PtrType end_;
};

using MemoryIterator = SafeGenericMemoryIterator<const byte*>;

/// Multipass bounded iterator reading from [`begin`, `end`).
///
/// Supports `size()`, `position()`, `rewind()`, and `seek()`.
///
/// Memory footprint: three pointers and a boolean.
///
/// Implements multipass input iterator contract.
template <typename PtrTypeT>
class MultipassGenericMemoryIterator {
 public:
  using PtrType = PtrTypeT;

  /// Creates multipass iterator over [`begin`, `end`).
  MultipassGenericMemoryIterator(PtrType begin, PtrType end)
      : ptr_(begin), position_(0), size_(end - begin), eos_(false) {}

  /// Reads one byte at current position and advances.
  ///
  /// If position is already at/after size, returns zero byte and sets EOS.
  byte read() {
    if (position_ >= size_) {
      eos_ = true;
      return byte{0};
    }
    return ptr_[position_++];
  }

  /// Reads up to `count` bytes from current position and advances.
  ///
  /// If already at/after size, returns zero and sets EOS.
  /// Reading exactly to end does not set EOS.
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

  /// Advances by up to `count` bytes.
  ///
  /// Advancing past end clamps to end and sets EOS.
  /// Advancing exactly to end keeps status `kOk`.
  void skip(size_t count) {
    if (position_ + count <= size_) {
      position_ += count;
    } else {
      position_ = size_;
      eos_ = true;
      return;
    }
  }

  /// Returns `kEndOfStream` after EOS is observed, otherwise `kOk`.
  Status status() const { return eos_ ? kEndOfStream : kOk; }

  /// Returns pointer at current position.
  PtrType ptr() const { return ptr_ + position_; }

  /// Returns total iterable size.
  uint64_t size() const { return size_; }

  /// Returns current position.
  uint64_t position() const { return position_; }

  /// Resets position to start and clears EOS.
  void rewind() {
    position_ = 0;
    eos_ = false;
  }

  /// Sets absolute position and clears EOS.
  ///
  /// Position is not range-checked here; out-of-range is detected by
  /// subsequent read/skip operations.
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

namespace internal {

template <typename Itr>
struct MemoryIteratorTraits {
  static constexpr bool is_memory = false;
};

template <typename PtrType>
struct MemoryIteratorTraits<UnsafeGenericMemoryIterator<PtrType>> {
  static constexpr bool is_memory = true;
};

template <typename PtrType>
struct MemoryIteratorTraits<SafeGenericMemoryIterator<PtrType>> {
  static constexpr bool is_memory = true;
};

template <typename PtrType>
struct MemoryIteratorTraits<MultipassGenericMemoryIterator<PtrType>> {
  static constexpr bool is_memory = true;
};

}  // namespace internal

}  // namespace roo_io
