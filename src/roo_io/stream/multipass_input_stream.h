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
  // If the status was not `kOk` and not `kEndOfStream` at the time of the
  // call, leaves it as is and returns zero.
  //
  // In case of error, updates the status accordingly, and returns zero.
  //
  virtual uint64_t size() = 0;

  // Returns the current byte offset relative to the beginning of the stream. If
  // the status is neither `kOk` nor `kEndOfStream`, can return an arbitrary
  // value.
  //
  virtual uint64_t position() const = 0;

  // Resets the stream to its starting position.
  //
  // If the status was neither 'kOk' nor 'kEndOfStream' prior to the call,
  // leaves it as is. In case of success, resets status to 'kOk' (potentially
  // clearing the end-of-stream status). On error, updates the status
  // accordingly.
  //
  // After a successful rewind, a call to `position()` should return zero.
  //
  virtual void rewind() { seek(0); }

  // Resets the stream to the specified byte offset relative to the starting
  // position. The offset may be past the current `size()`.
  //
  // If the status was neither 'kOk' nor 'kEndOfStream' prior to the call,
  // leaves it as is. In case of error, updates the status accordingly.
  //
  // On success, clears the end-of-stream status (updating status to 'kOk').
  //
  // After a successful seek, a call to `position()` should return value equal
  // to `position`.
  //
  virtual void seek(uint64_t offset) = 0;
};

}  // namespace roo_io
