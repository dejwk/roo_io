#pragma once

#include <inttypes.h>

#include "roo_io/byte.h"
#include "roo_io/status.h"

namespace roo_io {

// Virtualizes access to files, memory, or other sources. Represents an 'open'
// resource with a 'file pointer'.
class InputStream {
 public:
  virtual ~InputStream() { close(); }

  virtual bool isOpen() const = 0;

  virtual void close() {};

  // Reads up to `count` bytes into `result`, and updates `status()`. Returns
  // the number of bytes read, which must be greater than zero on success
  // (i.e. when `status()` returns 'kOk'), zero on end-of-stream (i.e. when
  // `status()` returns 'kEndOfStream'), and possibly zero on error (i.e.
  // when `status()` returns another value).
  //
  // If the status was not 'kOk' prior to the call, leaves it as is and returns
  // zero. Otherwise, if there are less then `count` but more than zero bytes
  // available before the end of stream, reads and returns the available bytes,
  // and leaves status as `kOk`. Otherwise if there are no more bytes available
  // before end of stream, returns zero and updates status to `kEndOfStream`.
  //
  // In case of error, updates the status, and returns the number of bytes that
  // have been read before the error was encountered (possibly zero, but might
  // be greater than zero).
  //
  virtual size_t read(byte* result, size_t count) = 0;

  // Reads the prescribed number of bytes, blocking if necessary.
  virtual size_t readFully(byte* buf, size_t count) {
    size_t read_total = 0;
    while (count > 0) {
      int read_now = read(buf, count);
      if (read_now == 0) break;
      buf += read_now;
      read_total += read_now;
      count -= read_now;
    }
    return read_total;
  }

  // Skips over the specified count of bytes. Updates the `status()`.
  //
  // If the status was not `kOk` prior to the call, leaves it as is. Otherwise,
  // if the skip was succesful and the stream is before or exactly at the end,
  // leaves the status as `kOk`. Otherwise, if the skip moves the stream past
  // the end, updates the status to `kEndOfStream`.
  //
  // In case of error, updates the status accordingly.
  //
  virtual void skip(uint64_t count) = 0;

  // Returns the status of the most recent I/O operation.
  virtual Status status() const = 0;
};

}  // namespace roo_io
