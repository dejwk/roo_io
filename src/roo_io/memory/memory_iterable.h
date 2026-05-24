#pragma once

#include <memory>

#include "roo_io/memory/memory_input_iterator.h"

namespace roo_io {

/// Wraps a raw memory pointer as an iterable unsafe memory reader.
template <typename PtrType>
class UnsafeGenericMemoryIterable {
 public:
  /// Stores the starting pointer used to create iterators.
  UnsafeGenericMemoryIterable(PtrType ptr) : ptr_(ptr) {}

  /// Returns a fresh iterator starting at the stored pointer.
  UnsafeGenericMemoryIterator<PtrType> iterator() const {
    return UnsafeGenericMemoryIterator<PtrType>(ptr_);
  }

 private:
  PtrType ptr_;
};

using UnsafeMemoryIterable = UnsafeGenericMemoryIterable<const byte*>;

/// Wraps a bounded memory range as an iterable safe memory reader.
template <typename PtrType>
class SafeGenericMemoryIterable {
 public:
  /// Stores the memory range used to create iterators.
  SafeGenericMemoryIterable(PtrType begin, PtrType end)
      : begin_(begin), end_(end) {}

  /// Returns a fresh iterator over the stored range.
  SafeGenericMemoryIterator<PtrType> iterator() const {
    return SafeGenericMemoryIterator<PtrType>(begin_, end_);
  }

 private:
  PtrType begin_;
  PtrType end_;
};

using MemoryIterable = SafeGenericMemoryIterable<const byte*>;

/// Wraps a bounded memory range as an iterable multipass memory reader.
template <typename PtrType>
class MultipassGenericMemoryIterable {
 public:
  /// Stores the memory range used to create iterators.
  MultipassGenericMemoryIterable(PtrType begin, PtrType end)
      : begin_(begin), end_(end) {}

  /// Returns a fresh multipass iterator over the stored range.
  MultipassGenericMemoryIterator<PtrType> iterator() const {
    return MultipassGenericMemoryIterator<PtrType>(begin_, end_);
  }

  /// Returns the total size of the stored range in bytes.
  uint64_t size() const { return end_ - begin_; }

 private:
  PtrType begin_;
  PtrType end_;
};

using MultipassMemoryIterable = MultipassGenericMemoryIterable<const byte*>;

}  // namespace roo_io