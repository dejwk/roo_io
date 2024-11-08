#pragma once

#include <stdio.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include "roo_io/fs/filesystem.h"
#include "roo_io/stream/output_stream.h"

namespace roo_io {

class PosixFileOutputStream : public OutputStream {
 public:
  PosixFileOutputStream(Status error) : file_(nullptr), size_(-1), status_(error) {}

  PosixFileOutputStream(FILE* file)
      : file_(file), size_(-1), status_(file_ != nullptr ? kOk : kClosed) {}

  int write(const uint8_t* buf, unsigned int count) override {
    if (status_ != kOk && status_ != kEndOfStream) return -1;
    int result = ::fwrite(buf, 1, count, file_);
    if (result == count) return result;
    if (ferror(file_)) {
      switch (errno) {
        case ENOMEM: {
          status_ = kOutOfMemory;
          break;
        }
        default:
          status_ = kUnknownIOError;
          break;
      }
    }
    if (result == 0 && count > 0 && feof(file_) != 0) {
      status_ = kEndOfStream;
      return 0;
    }
    return result;
  }

  bool isOpen() const override {
    return status_ == kOk || status_ == kEndOfStream;
  }

  void close() override {
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
  FILE* file_;
  mutable int64_t size_;
  mutable Status status_;
};

}  // namespace roo_io