#include "roo_io/fs/posix/config.h"

#if ROO_IO_FS_SUPPORT_POSIX

#pragma once

#include <stdio.h>

#include <memory>

#include "roo_io/core/multipass_input_stream.h"
#include "roo_io/fs/mount_impl.h"

namespace roo_io {

/// Multipass input stream wrapper around a POSIX `FILE*`.
class PosixFileInputStream : public MultipassInputStream {
 public:
  /// Creates a detached stream that reports `error` from `status()`.
  PosixFileInputStream(Status error);

  /// Wraps an already open POSIX file handle and retains the mount while open.
  PosixFileInputStream(std::shared_ptr<MountImpl> mount, FILE* file);

  /// Closes the file if needed.
  ~PosixFileInputStream();

  /// Reads up to `count` bytes from the file.
  size_t read(byte* buf, size_t count) override;

  /// Seeks to `offset` in the file.
  void seek(uint64_t offset) override;

  /// Skips forward by `count` bytes.
  void skip(uint64_t count) override;

  /// Returns the current file offset.
  uint64_t position() const override { return (::ftell(file_)); }

  /// Returns the total file size.
  uint64_t size() override;

  /// Returns whether the file stream is open.
  bool isOpen() const override;

  /// Closes the file and releases any retained mount reference.
  void close() override;

  /// Returns the current stream status.
  Status status() const override { return status_; }

 private:
  std::shared_ptr<MountImpl> mount_;
  FILE* file_;
  mutable Status status_;
};

}  // namespace roo_io

#endif  // ROO_IO_FS_SUPPORT_POSIX
