#pragma once

#include "roo_io/core/input_stream.h"
#include "roo_io/ringpipe/ringpipe.h"

namespace roo_io {

/// `InputStream` adapter that reads from `RingPipe`.
class RingPipeInputStream : public InputStream {
 public:
  /// Creates adapter over `pipe`.
  explicit RingPipeInputStream(RingPipe& pipe) : pipe_(pipe) {}

  /// Reads from ring pipe.
  ///
  /// Updates status indirectly (as observed via `status()`).
  ///
  /// @return Number of bytes read.
  size_t read(byte* data, size_t len) override { return pipe_.read(data, len); }

  /// Non-blocking read from ring pipe.
  ///
  /// Updates status indirectly (as observed via `status()`).
  ///
  /// @return Number of bytes read.
  size_t tryRead(byte* data, size_t len) override {
    return pipe_.tryRead(data, len);
  }

  /// Returns currently readable byte count.
  ///
  /// @return Bytes readable without blocking.
  size_t available() const { return pipe_.availableForRead(); }

  /// Returns current input-end status of underlying pipe.
  ///
  /// @return Current status.
  Status status() const override { return pipe_.inputStatus(); }

  /// Closes input end of underlying pipe.
  ///
  /// Updates status as observed via subsequent `status()` calls.
  void close() override { pipe_.closeInput(); }

 private:
  RingPipe& pipe_;
};

}  // namespace roo_io
