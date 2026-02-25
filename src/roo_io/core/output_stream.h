#pragma once

#include <cstddef>

#include "roo_io/base/byte.h"
#include "roo_io/status.h"

namespace roo_io {

/// Virtualizes access to writable sinks (files, memory, devices).
///
/// Represents an open writable resource.
///
/// For iterator-style writes, prefer `BufferedOutputStreamIterator` to reduce
/// virtual-call overhead per byte.
class OutputStream {
 public:
  virtual ~OutputStream() { close(); }

  /// Returns whether stream is considered open.
  ///
  /// @return `true` when `status()` is `kOk`.
  bool isOpen() const { return status() == kOk; }

  /// Flushes and closes this stream.
  ///
  /// On success, if status was `kOk`, status transitions to `kClosed`.
  /// On failure, status reflects encountered error.
  virtual void close() { flush(); }

  /// Attempts to write up to `count` bytes from `buf`.
  ///
  /// Updates status.
  ///
  /// Contract:
  /// - On success (`status() == kOk`), returns at least one byte.
  /// - On error, may return zero or number of bytes written before failure.
  ///
  /// If pre-call status is not `kOk`, call should have no effect and return
  /// zero.
  ///
  /// Successful return does not guarantee data reached final sink until
  /// `flush()` succeeds.
  ///
  /// @return Number of bytes written.
  virtual size_t write(const byte* buf, size_t count) = 0;

  /// Attempts to write without indefinite blocking.
  ///
  /// Updates status.
  ///
  /// Similar to `write()`, but may return zero on success if forward progress
  /// would require unacceptable blocking.
  ///
  /// Repeated `tryWrite()` calls make forward progress without requiring
  /// fallback to `write()`.
  ///
  /// @return Number of bytes written.
  virtual size_t tryWrite(const byte* buf, size_t count) {
    return write(buf, count);
  }

  /// Attempts to write `count` bytes from `buf`.
  ///
  /// Updates status via `write()` calls.
  ///
  /// Unlike `write()`, this method keeps writing until either:
  /// - all `count` bytes are written, or
  /// - an error occurs.
  ///
  /// If pre-call status is not `kOk`, this method returns zero and leaves
  /// status unchanged.
  ///
  /// Successful write still may be buffered; call `flush()` to force sink I/O.
  ///
  /// @return Total bytes written.
  virtual size_t writeFully(const byte* buf, size_t count) {
    size_t written_total = 0;
    while (count > 0) {
      size_t written_now = write(buf, count);
      if (written_now == 0) break;
      buf += written_now;
      written_total += written_now;
      count -= written_now;
    }
    return written_total;
  }

  /// Flushes buffered data to the underlying sink.
  ///
  /// May update `status()`. Stream is also flushed on destruction.
  virtual void flush() {}

  /// Returns underlying stream status.
  ///
  /// Updated by write/flush operations. Status is either `kOk` or an error
  /// (never `kEndOfStream`).
  virtual Status status() const = 0;
};

}  // namespace roo_io
