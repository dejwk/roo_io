#pragma once

#include <memory>

#include "roo_io/stream/random_access_input_stream.h"

namespace roo_io {

class Resource {
 public:
  Resource() = default;
  virtual ~Resource() = default;

  virtual std::unique_ptr<RandomAccessInputStream> open() const = 0;
};

}  // namespace roo_io