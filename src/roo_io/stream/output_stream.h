#pragma once

#include <inttypes.h>

#include "roo_io/status.h"

namespace roo_io {

// Virtualizes access to files, memory, or other sources. Represents an 'open'
// resource with a 'file pointer'.
class OutputStream {
 public:
  virtual ~OutputStream() { close(); }

  virtual bool isOpen() const = 0;

  virtual void close() { flush(); }

  // Tries to write up to count bytes. On success, returns the number of bytes
  // written. On failure, returns < 0.
  virtual int write(const uint8_t* buf, unsigned int count) = 0;

  virtual int writeFully(const uint8_t* buf, unsigned int count) {
    unsigned int written_total = 0;
    while (count > 0) {
      int written_now = write(buf, count);
      if (written_now <= 0) break;
      buf += written_now;
      written_total += written_now;
      count -= written_now;
    }
    return written_total;
  }

  virtual void flush() {}

  // Returns the status of the most recent I/O operation.
  virtual Status status() const = 0;
};

}  // namespace roo_io
