#pragma once

#include <memory>

#include "roo_io/core/input_stream.h"
#include "roo_io/core/multipass_input_stream.h"

namespace roo_io {

/// Abstract readable resource that can be opened as a single-pass stream.
class Resource {
 public:
  Resource() = default;
  virtual ~Resource() = default;

  /// Opens the resource for reading.
  virtual std::unique_ptr<InputStream> open() const = 0;
};

/// Abstract readable resource that can be opened as a seekable stream.
class MultipassResource {
 public:
  MultipassResource() = default;
  virtual ~MultipassResource() = default;

  /// Opens the resource for seekable reading.
  virtual std::unique_ptr<MultipassInputStream> open() const = 0;
};

}  // namespace roo_io