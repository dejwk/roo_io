#pragma once

#include <memory>

#include "roo_io/memory/memory_input_iterator.h"

namespace roo_io {

template <typename PtrType>
class UnsafeGenericMemoryIterable {
 public:
  UnsafeGenericMemoryIterable(PtrType ptr) : ptr_(ptr) {}

  UnsafeGenericMemoryIterator<PtrType> iterator() const {
    return UnsafeGenericMemoryIterator<PtrType>(ptr_);
  }

 private:
  PtrType ptr_;
};

using UnsafeMemoryIterable = UnsafeGenericMemoryIterable<const byte*>;

template <typename PtrType>
class SafeGenericMemoryIterable {
 public:
  SafeGenericMemoryIterable(PtrType begin, PtrType end)
      : begin_(begin), end_(end) {}

  SafeGenericMemoryIterator<PtrType> iterator() const {
    return SafeGenericMemoryIterator<PtrType>(begin_, end_);
  }

 private:
  PtrType begin_;
  PtrType end_;
};

using MemoryIterable = SafeGenericMemoryIterable<const byte*>;

template <typename PtrType>
class MultipassGenericMemoryIterable {
 public:
  MultipassGenericMemoryIterable(PtrType begin, PtrType end)
      : begin_(begin), end_(end) {}

  MultipassGenericMemoryIterator<PtrType> iterator() const {
    return MultipassGenericMemoryIterator<PtrType>(begin_, end_);
  }

  uint64_t size() const { return end_ - begin_; }

 private:
  PtrType begin_;
  PtrType end_;
};

using MultipassMemoryIterable = MultipassGenericMemoryIterable<const byte*>;

}  // namespace roo_io