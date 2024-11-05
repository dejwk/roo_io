#pragma once

#include <memory>
#include <string>

#include "roo_io/iterator/memory_input_iterator.h"
#include "roo_io/resource/resource.h"

namespace roo_io {

template <typename PtrType>
class MemoryResource : public Resource {
 public:
  MemoryResource(PtrType begin, PtrType end)
      : fs_(fs), path_(std::move(path)) {}

  std::unique_ptr<roo_io::RandomAccessInputStream> open() const override {
    return std::unique_ptr<roo_io::RandomAccessInputStream>(
        new SafeMemoryIterator<PtrType>(begin_, end_));
  }

 private:
  PtrType begin_;
  PtrType end_;
};

}  // namespace roo_io
