#pragma once

#include <stdio.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include "roo_io/core/output_stream.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

class PosixFileOutputStream : public OutputStream {
 public:
  PosixFileOutputStream(Status error);

  PosixFileOutputStream(std::shared_ptr<MountImpl> mount, FILE* file);

  ~PosixFileOutputStream();

  size_t write(const byte* buf, size_t count) override;

  void close() override;

  Status status() const override { return status_; }

 private:
  std::shared_ptr<MountImpl> mount_;
  FILE* file_;
  mutable int64_t size_;
  mutable Status status_;
};

}  // namespace roo_io