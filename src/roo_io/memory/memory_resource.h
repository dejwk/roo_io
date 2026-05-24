#pragma once

#include <memory>
#include <string>

#include "roo_io/core/resource.h"
#include "roo_io/memory/memory_input_stream.h"

namespace roo_io {

/// Multipass resource that reopens the same contiguous memory range each time.
template <typename PtrType>
class MemoryResource : public MultipassResource {
 public:
  /// Stores the backing memory range.
  MemoryResource(PtrType begin, PtrType end) : begin_(begin), end_(end) {}

  /// Opens a new multipass stream over the stored memory range.
  std::unique_ptr<roo_io::MultipassInputStream> open() const override {
    return std::unique_ptr<roo_io::MultipassInputStream>(
        new MemoryInputStream<PtrType>(begin_, end_));
  }

 private:
  PtrType begin_;
  PtrType end_;
};

}  // namespace roo_io
