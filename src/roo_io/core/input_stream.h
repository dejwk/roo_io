#pragma once

#include <inttypes.h>

#include "roo_io/base/byte.h"
#include "roo_io/status.h"

namespace roo_io {

/// Virtualizes access to files, memory, and other readable sources.
///
/// Represents an open resource with a read cursor.
///
/// For iterator-style usage, prefer `BufferedInputStreamIterator` to avoid
/// virtual-call overhead per byte.
class InputStream {
 public:
  virtual ~InputStream() { close(); }

  /// Returns whether stream is considered open.
  ///
  /// @return `true` when `status()` is `kOk` or `kEndOfStream`.
  virtual bool isOpen() const {
    Status s = status();
    return s == kOk || s == kEndOfStream;
  }

  /// Closes this stream.
  ///
  /// Updates status.
  ///
  /// If previous status was `kOk` or `kEndOfStream`, status transitions to
  /// `kClosed`.
  ///
  /// After close, read operations should return zero bytes.
  virtual void close() {}

  /// Attempts to read up to `count` bytes into `result`.
  ///
  /// Updates status.
  ///
  /// Contract:
  /// - On success (`status() == kOk`), returns at least one byte.
  /// - On end-of-stream (`status() == kEndOfStream`), returns zero.
  /// - On error, may return zero or number of bytes read before failure.
  ///
  /// If status before call is not `kOk`, leaves it unchanged and returns zero.
  ///
  /// Implementations may return fewer than `count` bytes even when more data is
  /// available. Use `readFully()` if that is not the desired behavior.
  /// @return Number of bytes read.
  virtual size_t read(byte* result, size_t count) = 0;

  /// Attempts to read up to `count` bytes into `result` without indefinitely
  /// blocking.
  ///
  /// Updates status.
  ///
  /// Similar to `read()`, but may return zero on success when no data is
  /// currently available without blocking.
  ///
  /// The implementation has some leeway for deciding what constitutes
  /// unacceptable blocking, as long as it is guaranteed that callers can always
  /// make progress by calling tryRead() repeatedly, and never calling read().
  ///
  /// @return Number of bytes read.
  virtual size_t tryRead(byte* result, size_t count) {
    return read(result, count);
  }

  /// Attempts to read `count` bytes into `buf`; blocks as needed.
  ///
  /// Updates status.
  ///
  /// Unlike `read()`, this method keeps reading until one of the following:
  /// - `count` bytes are read,
  /// - end-of-stream is reached,
  /// - an error occurs.
  ///
  /// If pre-call status is not `kOk`, this method returns zero and leaves
  /// status unchanged.
  ///
  /// @return Total bytes read.
  virtual size_t readFully(byte* buf, size_t count) {
    size_t read_total = 0;
    while (count > 0) {
      int read_now = read(buf, count);
      if (read_now == 0) break;
      buf += read_now;
      read_total += read_now;
      count -= read_now;
    }
    return read_total;
  }

  /// Skips over `count` bytes, updating `status()`.
  ///
  /// Conceptually equivalent to `readFully(tmp, count)` and discarding data.
  ///
  /// If pre-call status is not `kOk`, leaves status unchanged and returns.
  ///
  /// If skip ends exactly at stream end, status remains `kOk`.
  /// If skip ends before stream end, status remains `kOk`.
  /// If skip crosses stream end, status becomes `kEndOfStream`.
  ///
  /// Any I/O error updates status accordingly.
  virtual void skip(uint64_t count) {
    byte buf[64];
    while (count >= 64) {
      if (status() != kOk) return;
      count -= read(buf, 64);
    }
    while (count > 0) {
      if (status() != kOk) return;
      count -= read(buf, count);
    }
  }

  /// Returns status of the most recent I/O operation.
  virtual Status status() const = 0;
};

}  // namespace roo_io
