#pragma once

#include <memory>
#include <string>

#include "roo_io/memory/memory_input_stream.h"
#include "roo_io/core/resource.h"

namespace roo_io {

template <typename PtrType>
class MemoryResource : public MultipassResource {
 public:
  MemoryResource(PtrType begin, PtrType end)
      : fs_(fs), path_(std::move(path)) {}

  std::unique_ptr<roo_io::MultipassInputStream> open() const override {
    return std::unique_ptr<roo_io::MultipassInputStream>(
        new MemoryInputStream<PtrType>(begin_, end_));
  }

 private:
  PtrType begin_;
  PtrType end_;
};

}  // namespace roo_io
