#pragma once

#ifdef ARDUINO

#include <FS.h>

#include "roo_io/core/output_stream.h"
#include "roo_io/fs/mount_impl.h"

namespace roo_io {

/// Output stream wrapper around an Arduino `fs::File`.
class ArduinoFileOutputStream : public OutputStream {
 public:
  /// Creates a detached stream that reports `error` from `status()`.
  ArduinoFileOutputStream(Status error);

  /// Wraps an already open file when the filesystem lifetime is externally
  /// managed.
  ArduinoFileOutputStream(fs::File file);

  /// Wraps an already open file and keeps the mount alive while it is in use.
  ArduinoFileOutputStream(std::shared_ptr<MountImpl> mount, fs::File file);

  /// Writes up to `count` bytes to the file.
  size_t write(const byte* buf, size_t count) override;

  /// Flushes pending file data to the backing filesystem.
  void flush() override;

  /// Closes the file and releases any retained mount reference.
  void close() override;

  /// Returns the current stream status.
  Status status() const override { return status_; }

 private:
  std::shared_ptr<MountImpl> mount_;
  fs::File file_;
  Status status_;
};

}  // namespace roo_io

#endif  // ARDUINO
