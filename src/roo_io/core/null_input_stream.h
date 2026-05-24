#pragma once

#include "roo_io/core/multipass_input_stream.h"

namespace roo_io {

/// Multipass input stream that never yields data and only reports a preset
/// status.
class NullInputStream : public MultipassInputStream {
 public:
  /// Creates a detached null stream that reports `error` from `status()`.
  NullInputStream(Status error = kClosed) : status_(error) {}

  /// Returns `false`; null streams are never considered open.
  bool isOpen() const override { return false; }

  /// Leaves the configured status unchanged.
  void close() override {}

  /// Returns zero bytes without mutating the configured status.
  size_t read(byte* buf, size_t count) override { return 0; }

  /// Ignores skip requests.
  void skip(uint64_t count) override {}

  /// Returns the configured terminal status.
  Status status() const override { return status_; }

  /// Returns zero because the stream has no backing data.
  uint64_t size() override { return 0; }

  /// Returns zero because the stream never advances.
  uint64_t position() const override { return 0; }

  /// Ignores seek requests.
  void seek(uint64_t offset) override {}

 private:
  Status status_;
};

}  // namespace roo_io