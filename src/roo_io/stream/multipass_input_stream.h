#pragma once

#include <inttypes.h>

#include "roo_io/stream/input_stream.h"

namespace roo_io {

// Virtualizes access to files, memory, or other sources. Represents an 'open'
// resource with a 'file pointer'.
class MultipassInputStream : public InputStream {
 public:
  // Returns the total number of bytes comprising the stream, counting from the
  // beginning. The method can return different value each time it is called,
  // e.g. when the underlying data source is concurrently written to.
  //
  // If the status was not `kOk` and not `kEndOfStream` at the time of the call,
  // leaves it as is and returns zero.
  //
  // In case of error, updates the status accordingly, and returns zero.
  //
  virtual uint64_t size() = 0;

  // Returns the current position in the stream, in bytes. The stream must be
  // open.
  virtual uint64_t position() const = 0;

  // Returns true on success.
  virtual void seek(uint64_t offset) = 0;
};

}  // namespace roo_io
