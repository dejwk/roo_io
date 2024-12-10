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
//   // Since the data may be buffered, successful write does not guarantee that
//   // the data made it all the way through to the sink, until `flush()` is
//   // called.
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
//   // Since the data may be buffered, successful write does not guarantee that
//   // the data made it all the way through to the sink, until `flush()` is
//   // called.
//   //
//   size_t write(const byte* buf, size_t count);
//
//   // Ensures that any data that might be buffered by this iterator are
//   // written out to the underlying sink. May update the `status()`.
//   //
//   // The data gets automatically flushed when the iterator is destroyed.
//   //
//   void flush();
//
//   // Returns the status of the underlying stream. Updated by write
//   // operations. Always 'kOk' or failure (never 'kEndOfStream').
//   //
//   Status status() const;
// };
