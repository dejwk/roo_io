#pragma once

#include "roo_io/base/byte.h"
#include "roo_io/core/output_stream.h"

namespace roo_io {

/// Output stream that discards all writes and reports a preset status.
class NullOutputStream : public OutputStream {
 public:
  /// Creates a detached null stream that reports `error` from `status()`.
  NullOutputStream(Status error = kClosed) : status_(error) {}

  /// Leaves the configured status unchanged.
  void close() override {}

  /// Returns the configured terminal status.
  Status status() const override { return status_; }

  /// Accepts no data and always returns zero bytes written.
  size_t write(const byte* buf, size_t count) override { return 0; }

 private:
  Status status_;
};

}  // namespace roo_io