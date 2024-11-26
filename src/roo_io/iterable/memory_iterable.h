#pragma once

#include <memory>

#include "pgmspace.h"
#include "roo_io/iterator/memory_input_iterator.h"

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

using UnsafeMemoryIterable = UnsafeGenericMemoryIterable<const uint8_t*>;

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

using MemoryIterable = SafeGenericMemoryIterable<const uint8_t*>;

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

using MultipassMemoryIterable = MultipassGenericMemoryIterable<const uint8_t*>;

}  // namespace roo_io