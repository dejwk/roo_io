#pragma once

#include <inttypes.h>

#include "roo_io/stream/input_stream.h"

namespace roo_io {

// Virtualizes access to files, memory, or other sources. Represents an 'open'
// resource with a 'file pointer'.
class MultipassInputStream : public InputStream {
 public:
  // Returns the size of the resource in bytes. The stream must be open.
  virtual uint64_t size() const = 0;

  // Returns the current position in the stream, in bytes. The stream must be
  // open.
  virtual uint64_t position() const = 0;

  // Returns true on success.
  virtual bool seek(uint64_t offset) = 0;

  // Returns true on success.
  bool skip(uint64_t count) override {
    return seek(position() + count);
  }
};

}  // namespace roo_io
