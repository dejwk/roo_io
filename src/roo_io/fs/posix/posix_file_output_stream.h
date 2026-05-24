#pragma once

#include "roo_io/fs/posix/config.h"

#if ROO_IO_FS_SUPPORT_POSIX

#include <stdio.h>
#include <sys/stat.h>

#include "roo_io/core/output_stream.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

/// Output stream wrapper around a POSIX `FILE*`.
class PosixFileOutputStream : public OutputStream {
 public:
  /// Creates a detached stream that reports `error` from `status()`.
  PosixFileOutputStream(Status error);

  /// Wraps an already open POSIX file handle and retains the mount while open.
  PosixFileOutputStream(std::shared_ptr<MountImpl> mount, FILE* file);

  /// Closes the file if needed.
  ~PosixFileOutputStream();

  /// Writes up to `count` bytes to the file.
  size_t write(const byte* buf, size_t count) override;

  /// Closes the file and releases any retained mount reference.
  void close() override;

  /// Returns the current stream status.
  Status status() const override { return status_; }

 private:
  std::shared_ptr<MountImpl> mount_;
  FILE* file_;
  mutable int64_t size_;
  mutable Status status_;
};

}  // namespace roo_io

#endif  // ROO_IO_FS_SUPPORT_POSIX
