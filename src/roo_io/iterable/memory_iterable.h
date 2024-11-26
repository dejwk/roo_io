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

// using DramPtr = UnsafeMemoryIterable<uint8_t*>;
// using ConstDramPtr = UnsafeMemoryIterable<const uint8_t*>;
// using ProgMemPtr = UnsafeMemoryIterable<const uint8_t PROGMEM*>;

}  // namespace roo_io