#pragma once

#include <cstdint>

#include "roo_io/byte.h"
#include "roo_io/status.h"

// Input iterators define a template contract for streams of bytes, to be used
// for performance-critical applications where it is desirable that byte reads
// are inlined. An example application is to read byte-by-byte from a buffered
// file stream.

// The baseline 'input iterator' contract is as follows:
//
// class MyInputIterator {
//  public:
//   // Iterator must be movable.
//   //
//   // MyInputIterator(MyInputIterator&& other);
//
//   // Reads a single byte, and updates `status()`.
//   // The caller can rely on the returned value only if `status()` is `kOk`.
//   // If the status was not 'kOk' prior to the call, leaves it as, is and
//   // returns an arbitrary value. Otherwise, if it fails, updates the status
//   // and returns an arbitrary value.
//   //
//   byte read();
//
//   // Reads up to `count` bytes into `result`, and updates `status()`. Returns
//   // the number of bytes read, which must be greater than zero on success
//   // (i.e. when `status()` returns `kOk`), zero on end-of-stream (i.e. when
//   // `status()` returns `kEndOfStream`), and possibly zero on error (i.e.
//   // when `status()` returns another value).
//   //
//   // If the status was not 'kOk' prior to the call, leaves it as is and
//   // returns zero. Otherwise, if there are less then `count` but more than
//   // zero bytes available before the end of stream, reads and returns the
//   // available bytes, and leaves status as `kOk`. Otherwise if there are no
//   // more bytes available before end of stream, returns zero and updates
//   // status to `kEndOfStream`.
//   //
//   // In case of error, updates the status, and returns the number of bytes
//   // that have been read before the error was encountered (possibly zero, but
//   // might be greater than zero).
//   //
//   size_t read(byte* result, size_t count);
//
//   // Skips over the specified count of bytes. Updates the `status()`.
//   //
//   // If the status was not `kOk` prior to the call, leaves it as is.
//   // Otherwise, if the skip was succesful and the stream is before or exactly
//   // at the end, leaves the status as `kOk`. Otherwise, if the skip moves the
//   // stream past the end, updates the status to `kEndOfStream`.
//   //
//   // In case of error, updates the status accordingly.
//   //
//   // The behavior of skip should be equivalent to:
//   // while (count-- > 0) read();
//   //
//   void skip(size_t count);
//
//   // Returns `kOk` if the last `read` or `skip` operation succeeded. Returns
//   // `kEndOfStream` if the last operation moved the stream past the end.
//   // Returns a different status code, as appropriate, when the last operation
//   // failed.
//   Status status() const;
// };

// The more advanced 'multipass input iterator' contract is as follows:
//
// class MyMultipassInputIterator {
//  public:
//   // Iterator must be movable.
//   //
//   MyMultipassInputIterator(MyMultipassInputIterator&& other);
//
//   // Same as in the 'input iterator' above.
//   //
//   // Successful read advances the position pointer. Read that encounters end
//   // of stream leaves the position pointer unchanged.
//   byte read();
//
//   // Same as in the 'input iterator' above.
//   //
//   // In case of success or end-of-stream, position pointer is advanced by the
//   // number of elements read.
//   size_t read(byte* result, size_t count);
//
//   // Same as in the 'input iterator' above.
//   //
//   void skip(size_t count);
//
//   // Same as in the 'input iterator' above.
//   //
//   Status status() const;
//
//   // Returns the total number of bytes comprising the stream, counting from
//   // the beginning. The method can return different value each time it is
//   // called, e.g. when the underlying data source is concurrently written to.
//   //
//   // If the status was not `kOk` and not `kEndOfStream` at the time of the
//   // call, leaves it as is and returns zero.
//   //
//   // In case of error, updates the status accordingly, and returns zero.
//   //
//   uint64_t size();
//
//   // Returns the current byte offset relative to the beginning of the stream.
//   // If the status is neither `kOk` nor `kEndOfStream`, can return an
//   // arbitrary value.
//   //
//   uint64_t position() const;
//
//   // Resets the stream to its starting position.
//   //
//   // If the status was neither 'kOk' nor 'kEndOfStream' prior to the call,
//   // leaves it as is. In case of success, resets status to 'kOk'
//   // (potentially clearing the end-of-stream status). On error, updates the
//   // status accordingly.
//   //
//   void rewind();
//
//   // Resets the stream to the specified byte offset relative to the starting
//   // position. The offset may be past the current `size()`.
//   //
//   // If the status was neither 'kOk' nor 'kEndOfStream' prior to the call,
//   // leaves it as is. In case of error, updates the status accordingly.
//   //
//   // On success, clears the end-of-stream status (updating status to 'kOk').
//   //
//   // After a successful seek, a call to `position()` should return value
//   // equal to `position`.
//   //
//   void seek(uint64_t position);
// };