#pragma once

#include <stdint.h>

#include "roo_io/base/byte.h"
#include "roo_io/status.h"

/// Output iterator contract for byte sinks.
///
/// Use this in performance-critical paths where byte-level writes are inlined.
///
/// Baseline contract:
/// @code
/// class MyOutputIterator {
///  public:
///   // Iterator is movable.
///   // MyOutputIterator(MyOutputIterator&& other);
///
///   // Writes one byte and updates status().
///   // If pre-call status != kOk, call has no effect.
///   // On success, status remains kOk.
///   void write(byte v);
///
///   // Writes up to count bytes and updates status().
///   // Returns >0 on success (kOk), 0 or more on error (bytes written before
///   // failure).
///   // If pre-call status != kOk, call has no effect and return value is 0.
///   size_t write(const byte* buf, size_t count);
///
///   // Flushes buffered data to sink and updates status().
///   // If pre-call status != kOk, call may be treated as no-op.
///   void flush();
///
///   // Returns current status.
///   // Value is kOk or an error (never kEndOfStream).
///   Status status() const;
/// };
/// @endcode
