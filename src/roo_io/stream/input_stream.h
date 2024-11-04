#pragma once

#include <inttypes.h>

#include "roo_io/status.h"

namespace roo_io {

// Virtualizes access to files, memory, or other sources. Represents an 'open'
// resource with a 'file pointer'.
class InputStream {
 public:
  virtual ~InputStream() { close(); }

  virtual bool isOpen() const = 0;

  virtual void close() {};

  // Tries to read at least one byte, blocking if necessary. Returns zero on
  // EOF, negative value on error, and the number of bytes read otherwise.
  virtual int read(uint8_t* buf, unsigned int count) = 0;

  // Reads the prescribed number of bytes, blocking if necessary.
  virtual int readFully(uint8_t* buf, unsigned int count) {
    unsigned int read_total = 0;
    while (count > 0) {
      int read_now = read(buf, count);
      if (read_now <= 0) break;
      buf += read_now;
      read_total += read_now;
      count -= read_now;
    }
    return read_total;
  }

  // Returns true on success.
  virtual bool skip(uint64_t count) = 0;

  // Returns the status of the most recent I/O operation.
  virtual Status status() const = 0;
};

}  // namespace roo_io
