#pragma once

#if (defined ARDUINO)

#include <Arduino.h>

#include "roo_io/core/output_stream.h"

namespace roo_io {

/// Adapter exposing Arduino `Stream` as `OutputStream`.
///
/// Stream remains open until `close()` is called.
class ArduinoStreamOutputStream : public OutputStream {
 public:
  /// Creates adapter over Arduino `Print`.
  ArduinoStreamOutputStream(Stream& output);

  /// Non-blocking write.
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

  /// Writes exactly `count` bytes unless stream becomes unavailable.
  ///
  /// Updates status.
  ///
  /// @return Number of bytes written.
  size_t writeFully(const byte* buf, size_t count) override;

  /// Flushes buffered output to sink.
  ///
  /// Updates status.
  void flush() override;

  /// Closes adapter by setting status to `kClosed`.
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