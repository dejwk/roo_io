#include <cstdint>

#include "roo_io/base/byte.h"
#include "roo_io/core/input_iterator.h"
#include "roo_io/status.h"

/// Multipass input iterator contract (extends input iterator contract).
///
/// Baseline contract additions:
/// @code
/// class MyMultipassInputIterator {
///  public:
///   // Iterator is movable.
///   MyMultipassInputIterator(MyMultipassInputIterator&& other);
///
///   // Same read/read(skip)/status semantics as input iterator.
///   // Successful reads advance position.
///   byte read();
///   size_t read(byte* result, size_t count);
///   void skip(size_t count);
///   Status status() const;
///
///   // Returns stream size in bytes from beginning.
///   // Updates status.
///   // Value may change across calls for concurrently mutating sources.
///   // If pre-call status is neither kOk nor kEndOfStream, status is unchanged
///   // and return value is 0.
///   uint64_t size();
///
///   // Returns current byte offset from beginning.
///   // Does not modify status.
///   // If status is neither kOk nor kEndOfStream, return value is unspecified.
///   uint64_t position() const;
///
///   // Rewinds to beginning.
///   // Updates status.
///   // If pre-call status is neither kOk nor kEndOfStream, status is
///   // unchanged.
///   // On success, status becomes kOk and position() == 0.
///   void rewind();
///
///   // Seeks to byte offset from beginning (offset may exceed current size()).
///   // Updates status.
///   // If pre-call status is neither kOk nor kEndOfStream, status is
///   // unchanged.
///   // On success, status becomes kOk and position() == requested offset.
///   void seek(uint64_t position);
/// @endcode
/// };