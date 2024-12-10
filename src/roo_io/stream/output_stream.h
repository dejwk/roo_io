#pragma once

#include <inttypes.h>

#include "roo_io/status.h"

namespace roo_io {

// Virtualizes access to files, memory, or other sources that can be written to.
class OutputStream {
 public:
  virtual ~OutputStream() { close(); }

  // Equivalent to 'status() == kOk`.
  bool isOpen() const { return status() == kOk; }

  // Flushes and closes the stream. If successful, and if the state was 'kOk',
  // it is changed to 'kClosed'. Otherwise, the state is set to reflect the
  // failure encountered.
  virtual void close() { flush(); }

  // Attempts to write up to `count` bytes from the `buffer`. Updates
  // `status()`. Returns the number of bytes written, which must be greater than
  // zero on success (i.e. when `status()` returns `kOk`), and possibly zero on
  // error.
  //
  // If the status is not `kOk` before the call, the call has no effect and
  // returns zero.
  //
  // In case of error, updates the status, and returns the number of bytes that
  // have been written before the error was encountered (possibly zero, but
  // might be greater than zero).
  //
  // Since the data may be buffered, successful write does not guarantee that
  // the data made it all the way through to the sink, until `flush()` is
  // called.
  virtual size_t write(const byte* buf, size_t count) = 0;

  // Attempts to write `count` bytes from the `buffer`. Updates `status()`.
  // Returns the number of bytes written, which must be `count` on success (i.e.
  // when `status()` returns `kOk`), and possibly zero on error.
  //
  // This method is similar to `write()`, except it never returns fewer bytes
  // than `count`, unless it encounters an error.
  //
  // Since the data may be buffered, successful write does not guarantee that
  // the data made it all the way through to the sink, until `flush()` is
  // called.
  virtual size_t writeFully(const byte* buf, size_t count) {
    size_t written_total = 0;
    while (count > 0) {
      size_t written_now = write(buf, count);
      if (written_now == 0) break;
      buf += written_now;
      written_total += written_now;
      count -= written_now;
    }
    return written_total;
  }

  // Ensures that any data that might be buffered by this stream are written out
  // to the underlying sink. May update the `status()`.
  //
  // The stream gets automatically flushed when destroyed.
  virtual void flush() {}

  // Returns the status of the underlying stream. Updated by write operations.
  // Always 'kOk' or failure (never 'kEndOfStream').
  //
  virtual Status status() const = 0;
};

}  // namespace roo_io
