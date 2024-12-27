#pragma once

#include <stdio.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include "roo_io/core/output_stream.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

class PosixFileOutputStream : public OutputStream {
 public:
  PosixFileOutputStream(Status error)
      : file_(nullptr), size_(-1), status_(error) {}

  PosixFileOutputStream(std::shared_ptr<MountImpl> mount, FILE* file)
      : mount_(std::move(mount)),
        file_(file),
        size_(-1),
        status_(file_ != nullptr ? kOk : kClosed) {}

  ~PosixFileOutputStream() { ::fclose(file_); }

  size_t write(const byte* buf, size_t count) override {
    if (status_ != kOk) return 0;
    size_t result = ::fwrite(buf, 1, count, file_);
    if (result == count) return result;
    if (ferror(file_)) {
      mount_.reset();
      switch (errno) {
        case ENOMEM: {
          status_ = kOutOfMemory;
          break;
        }
        case ENOSPC: {
          status_ = kNoSpaceLeftOnDevice;
          break;
        }
        default:
          status_ = kUnknownIOError;
          break;
      }
    } else {
      status_ = kUnknownIOError;
    }
    return result;
  }

  void close() override {
    mount_.reset();
    if (status_ != kOk && status_ != kEndOfStream) return;
    if (::fclose(file_) == 0) {
      status_ = kClosed;
      return;
    }
    switch (errno) {
      case ENOSPC:
        status_ = kNoSpaceLeftOnDevice;
        break;
      default:
        status_ = kUnknownIOError;
    }
  }

  Status status() const override { return status_; }

 private:
  std::shared_ptr<MountImpl> mount_;
  FILE* file_;
  mutable int64_t size_;
  mutable Status status_;
};

}  // namespace roo_io