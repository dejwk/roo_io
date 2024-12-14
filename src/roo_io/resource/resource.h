#pragma once

#include <memory>

#include "roo_io/stream/input_stream.h"
#include "roo_io/stream/multipass_input_stream.h"

namespace roo_io {

class Resource {
 public:
  Resource() = default;
  virtual ~Resource() = default;

  virtual std::unique_ptr<InputStream> open() const = 0;
};

class MultipassResource {
 public:
  MultipassResource() = default;
  virtual ~MultipassResource() = default;

  virtual std::unique_ptr<MultipassInputStream> open() const = 0;
};

}  // namespace roo_io