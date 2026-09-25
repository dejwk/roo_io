#pragma once

#include "roo_io/core/input_stream.h"

namespace roo_io {

/// Borrowed input-stream window that exposes at most `limit` source bytes.
///
/// Closing or destroying this adapter never closes the borrowed source stream.
class LimitedInputStream : public InputStream {
 public:
  /// Creates a byte-bounded window over `input` without reading it.
  LimitedInputStream(InputStream& input, size_t limit)
      : input_(input), remaining_(limit), status_(kOk) {}

  LimitedInputStream(const LimitedInputStream&) = delete;
  LimitedInputStream& operator=(const LimitedInputStream&) = delete;
  LimitedInputStream(LimitedInputStream&&) = delete;
  LimitedInputStream& operator=(LimitedInputStream&&) = delete;

  /// Closes this adapter without closing its borrowed source stream.
  void close() override { status_ = kClosed; }

  /// Reads up to `count` bytes without crossing the window boundary.
  size_t read(byte* result, size_t count) override {
    if (count == 0) return 0;
    if (status() != kOk) return 0;
    if (remaining_ == 0) {
      status_ = kEndOfStream;
      return 0;
    }
    if (count > remaining_) count = remaining_;
    size_t read = input_.read(result, count);
    remaining_ -= read;
    if (read == 0 && input_.status() == kOk) status_ = kReadError;
    return read;
  }

  /// Attempts a nonblocking bounded read.
  size_t tryRead(byte* result, size_t count) override {
    if (count == 0) return 0;
    if (status() != kOk) return 0;
    if (remaining_ == 0) {
      status_ = kEndOfStream;
      return 0;
    }
    if (count > remaining_) count = remaining_;
    size_t read = input_.tryRead(result, count);
    remaining_ -= read;
    return read;
  }

  /// Skips up to `count` bytes without crossing the window boundary.
  void skip(uint64_t count) override {
    byte scratch[64];
    while (count > 0 && status() == kOk) {
      size_t chunk = count < sizeof(scratch) ? static_cast<size_t>(count)
                                             : sizeof(scratch);
      size_t read = this->read(scratch, chunk);
      if (read == 0) return;
      count -= read;
    }
  }

  /// Returns the remaining bytes available through this window.
  size_t remaining() const { return remaining_; }

  /// Returns the local boundary failure or the source stream status.
  Status status() const override {
    return status_ == kOk ? input_.status() : status_;
  }

 private:
  InputStream& input_;
  size_t remaining_;
  Status status_;
};

}  // namespace roo_io
