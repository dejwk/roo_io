#pragma once

#ifdef ARDUINO

#include <FS.h>

#include "roo_io/core/multipass_input_stream.h"
#include "roo_io/fs/mount_impl.h"

namespace roo_io {

/// Multipass input stream wrapper around an Arduino `fs::File`.
class ArduinoFileInputStream : public MultipassInputStream {
 public:
  /// Creates a detached stream that reports `error` from `status()`.
  ArduinoFileInputStream(Status error);

  /// Wraps an already open file when the filesystem lifetime is externally
  /// managed.
  ArduinoFileInputStream(fs::File file);

  /// Wraps an already open file and keeps the mount alive while it is in use.
  ArduinoFileInputStream(std::shared_ptr<MountImpl> mount, fs::File file);

  /// Reads up to `count` bytes from the file.
  size_t read(byte* buf, size_t count) override;

  /// Seeks to `offset` in the file.
  void seek(uint64_t offset) override;

  /// Skips forward by `count` bytes.
  void skip(uint64_t count) override;

  /// Returns the current file offset.
  uint64_t position() const override { return file_.position(); }

  /// Returns the total file size in bytes.
  uint64_t size() override { return file_.size(); }

  /// Returns whether the wrapped file handle is open.
  bool isOpen() const override { return file_.operator bool(); }

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
