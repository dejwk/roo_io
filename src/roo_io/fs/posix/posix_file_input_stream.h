#include "roo_io/fs/posix/config.h"

#if ROO_IO_FS_SUPPORT_POSIX

#pragma once

#include <memory>
#include <stdio.h>

#include "roo_io/core/multipass_input_stream.h"
#include "roo_io/fs/mount_impl.h"

namespace roo_io {

class PosixFileInputStream : public MultipassInputStream {
 public:
  PosixFileInputStream(Status error);

  PosixFileInputStream(std::shared_ptr<MountImpl> mount, FILE* file);

  ~PosixFileInputStream();

  size_t read(byte* buf, size_t count) override;

  void seek(uint64_t offset) override;

  void skip(uint64_t count) override;

  uint64_t position() const override { return (::ftell(file_)); }

  uint64_t size() override;

  bool isOpen() const override;

  void close() override;

  Status status() const override { return status_; }

 private:
  std::shared_ptr<MountImpl> mount_;
  FILE* file_;
  mutable Status status_;
};

}  // namespace roo_io

#endif  // ROO_IO_FS_SUPPORT_POSIX
