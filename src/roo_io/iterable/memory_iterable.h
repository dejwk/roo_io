#pragma once

#include <memory>

#include "pgmspace.h"
#include "roo_io/iterator/memory_input_iterator.h"

namespace roo_io {

template <typename PtrType>
class UnsafeMemoryIterable {
 public:
  UnsafeMemoryIterable(PtrType ptr) : ptr_(ptr) {}

  UnsafeMemoryIterator<PtrType> iterator() const {
    return UnsafeMemoryIterator<PtrType>(ptr_);
  }

 private:
  PtrType ptr_;
};

using DramPtr = UnsafeMemoryIterable<uint8_t*>;
using ConstDramPtr = UnsafeMemoryIterable<const uint8_t*>;
using ProgMemPtr = UnsafeMemoryIterable<const uint8_t PROGMEM*>;

}  // namespace roo_io