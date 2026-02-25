#pragma once

#if (defined ARDUINO)

#include <Stream.h>

#include "roo_io/core/input_stream.h"

namespace roo_io {

/// Adapter exposing Arduino `Stream` as `InputStream`.
///
/// Stream remains open until `close()` is called.
class ArduinoStreamInputStream : public InputStream {
 public:
  /// Creates adapter over Arduino `Stream`.
  ArduinoStreamInputStream(Stream& input);

  /// Non-blocking read.
  ///
  /// Updates status.
  ///
  /// @return Number of bytes read.
  size_t tryRead(byte* buf, size_t count) override;

  /// Potentially blocking read.
  ///
  /// Updates status.
  ///
  /// @return Number of bytes read.
  size_t read(byte* buf, size_t count) override;

  /// Reads exactly `count` bytes unless stream becomes unavailable.
  ///
  /// Updates status.
  ///
  /// @return Number of bytes read.
  size_t readFully(byte* buf, size_t count) override;

  /// Returns whether stream is considered open.
  ///
  /// @return `true` when `status()` is `kOk` or `kEndOfStream`.
  bool isOpen() const override;

  /// Closes adapter by setting status to `kClosed`.
  ///
  /// Updates status.
  void close() override { status_ = kClosed; }

  /// Returns current status.
  ///
  /// @return Current status value.
  Status status() const override { return status_; }

 private:
  Stream& input_;
  mutable Status status_;
};

}  // namespace roo_io

#endif  // (defined ARDUINO)