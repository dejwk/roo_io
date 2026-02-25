#pragma once

#include <cstdint>

#include "roo_io/base/byte.h"
#include "roo_io/status.h"

/// Input iterator contract for byte streams.
///
/// Use this in performance-critical paths where byte-level operations are
/// inlined.
///
/// Baseline contract:
/// @code
/// class MyInputIterator {
///  public:
///   // Iterator is movable.
///   // MyInputIterator(MyInputIterator&& other);
///
///   // Reads one byte and updates status().
///   // Returned byte is meaningful only when status() == kOk.
///   // If pre-call status != kOk, status is unchanged and return value is
///   // unspecified.
///   byte read();
///
///   // Reads up to count bytes into result and updates status().
///   // Returns >0 on success (kOk), 0 at EOS (kEndOfStream), and 0 or more on
///   // error depending on bytes read before failure.
///   // If pre-call status != kOk, status is unchanged and return value is 0.
///   size_t read(byte* result, size_t count);
///
///   // Skips count bytes and updates status().
///   // If pre-call status != kOk, status is unchanged.
///   // Reaching exactly the end keeps status() == kOk.
///   // Crossing past end sets status() == kEndOfStream.
///   // Equivalent behavior: while (count-- > 0) read();
///   void skip(size_t count);
///
///   // Returns current status; does not modify status.
///   // Typical values: kOk for success, kEndOfStream after crossing stream
///   // end, or an error status after failure.
///   Status status() const;
/// };
/// @endcode
