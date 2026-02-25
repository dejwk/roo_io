#pragma once

#include "roo_io/core/output_stream.h"
#include "roo_io/ringpipe/ringpipe.h"

namespace roo_io {

/// `OutputStream` adapter that writes to `RingPipe`.
class RingPipeOutputStream : public OutputStream {
 public:
  /// Creates adapter over `pipe`.
  explicit RingPipeOutputStream(RingPipe& pipe) : pipe_(pipe) {}

  /// Writes to ring pipe.
  ///
  /// Updates status indirectly (as observed via `status()`).
  ///
  /// @return Number of bytes written.
  size_t write(const byte* data, size_t len) override {
    return pipe_.write(data, len);
  }

  /// Non-blocking write to ring pipe.
  ///
  /// Updates status indirectly (as observed via `status()`).
  ///
  /// @return Number of bytes written.
  size_t tryWrite(const byte* data, size_t len) override {
    return pipe_.tryWrite(data, len);
  }

  /// Returns currently writable byte count.
  ///
  /// @return Bytes writable without blocking.
  size_t availableForWrite() const { return pipe_.availableForWrite(); }

  /// Returns current output-end status of underlying pipe.
  ///
  /// @return Current status.
  Status status() const override { return pipe_.outputStatus(); }

  /// Closes output end of underlying pipe.
  ///
  /// Updates status as observed via subsequent `status()` calls.
  void close() override { pipe_.closeOutput(); }

 private:
  RingPipe& pipe_;
};

}  // namespace roo_io