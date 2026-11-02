#pragma once

#if (defined ARDUINO)

#include <Arduino.h>

#include "roo_io/core/output_stream.h"

namespace roo_io {

/// Adapter exposing Arduino `Stream` as `OutputStream`.
///
/// Borrows the underlying Stream, which must outlive this adapter.
/// close() closes only this view. Serialize access to the Stream externally.
class ArduinoStreamOutputStream : public OutputStream {
 public:
  /// Creates an adapter over the borrowed Arduino `Stream`.
  ArduinoStreamOutputStream(Stream& output);

  /// Flushes this view before its borrowed Stream is released by the caller.
  ~ArduinoStreamOutputStream() override { close(); }

  /// Writes the currently available prefix without waiting for capacity.
  ///
  /// Updates status.
  ///
  /// @return Number of bytes written.
  size_t tryWrite(const byte* buf, size_t count) override;

  /// Potentially blocking write.
  ///
  /// Updates status.
  ///
  /// @return Number of bytes written.
  size_t write(const byte* buf, size_t count) override;

  /// Repeats partial writes until `count` bytes transfer or an error occurs.
  /// Zero progress is kWriteError; a zero-length request has no side effects.
  ///
  /// Updates status.
  ///
  /// @return Number of bytes written.
  size_t writeFully(const byte* buf, size_t count) override;

  /// Flushes the borrowed Stream and records any write error.
  ///
  /// Completion and blocking follow Stream::flush(). In particular, an
  /// Arduino hardware serial stream may wait until transmission completes.
  void flush() override;

  /// Flushes and closes this view, preserving any existing or flush error.
  ///
  /// Updates status.
  void close() override;

  /// Returns current status.
  ///
  /// @return Current status value.
  Status status() const override { return status_; }

 private:
  Stream& output_;
  mutable Status status_;
};

}  // namespace roo_io

#endif  // (defined ARDUINO)