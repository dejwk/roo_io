#include <cstdint>

#include "roo_io/base/byte.h"
#include "roo_io/core/input_iterator.h"
#include "roo_io/status.h"

// Input iterators define a template contract for streams of bytes, to be used
// for performance-critical applications where it is desirable that byte reads
// are inlined. An example application is to read byte-by-byte from a buffered
// file stream.

// The 'multipass input iterator' builds upon the 'input iterator':
//
// class MyMultipassInputIterator {
//  public:
//   // Iterator must be movable.
//   //
//   MyMultipassInputIterator(MyMultipassInputIterator&& other);
//
//   // Same as in the 'input iterator'.
//   //
//   // Successful read advances the position pointer. Read that encounters end
//   // of stream leaves the position pointer unchanged.
//   byte read();
//
//   // Same as in the 'input iterator'.
//   //
//   // In case of success or end-of-stream, position pointer is advanced by the
//   // number of elements read.
//   size_t read(byte* result, size_t count);
//
//   // Same as in the 'input iterator'.
//   //
//   void skip(size_t count);
//
//   // Same as in the 'input iterator'.
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
//   // After a successful rewind, a call to `position()` should return zero.
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