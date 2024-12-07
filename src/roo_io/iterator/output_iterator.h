#pragma once

#include <stdint.h>

#include "roo_io/byte.h"
#include "roo_io/status.h"

// Output iterators define a template contract for writing streams of bytes, to
// be used for performance-critical applications where it is desirable that byte
// writes are inlined. An example application is to write byte-by-byte to a
// buffered file stream.

// class MyOutputIterator {
//  public:
//   // Output iterator must be movable.
//   //
//   // MyOutputIterator(MyOutputIterator&& other);
//
//   // Writes a single byte. Updates `status()`.
//   //
//   // If the status is not `kOk` before the call, the call has no effect.
//   //
//   void write(byte v);
//
//   // Writes up to `count` bytes from the `buffer`. Updates `status()`.
//   // Returns the number of bytes written, which must be greater than zero on
//   // success (i.e. when `status()` returns `kOk`), and possibly zero on
//   // error.
//   //
//   // If the status is not `kOk` before the call, the call has no effect and
//   // returns zero.
//   //
//   // In case of error, updates the status, and returns the number of bytes
//   // that have been written before the error was encountered (possibly zero,
//   // but might be greater than zero).
//   //
//   size_t write(const byte* buf, size_t count);
//
//   // Returns the status of the underlying stream. Updated by write
//   // operations. Always 'kOk' or failure (never 'kEndOfStream').
//   //
//   Status status() const;
// };
