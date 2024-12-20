#pragma once

#include <stdio.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include "roo_io/fs/filesystem.h"
#include "roo_io/core/multipass_input_stream.h"

namespace roo_io {

class PosixFileInputStream : public MultipassInputStream {
 public:
  PosixFileInputStream(Status error) : file_(nullptr), status_(error) {}

  PosixFileInputStream(FILE* file)
      : file_(file), status_(file_ != nullptr ? kOk : kClosed) {}

  ~PosixFileInputStream() {
    if (file_ != nullptr) ::fclose(file_);
  }

  size_t read(byte* buf, size_t count) override {
    if (status_ != kOk) return 0;
    int result = fread(buf, 1, count, file_);
    if (result > 0) return result;
    if (ferror(file_) != 0) {
      switch (errno) {
        case ENOMEM: {
          status_ = kOutOfMemory;
          break;
        }
        default:
          status_ = kUnknownIOError;
          break;
      }
      return 0;
    }
    if (feof(file_) != 0) {
      status_ = kEndOfStream;
    } else {
      status_ = kUnknownIOError;
    }
    return 0;
  }

  void seek(uint64_t offset) override {
    if (status_ != kOk && status_ != kEndOfStream) return;
    if (::fseek(file_, offset, SEEK_SET) == 0) {
      status_ = kOk;
      return;
    }
    switch (errno) {
      case EFBIG:
      case EINVAL:
      case EOVERFLOW:
        status_ = kSeekError;
        break;
      default:
        status_ = kUnknownIOError;
        break;
    }
  }

  void skip(uint64_t count) override {
    if (status_ != kOk) return;
    if (::fseek(file_, count, SEEK_CUR) == 0) {
      if (position() > size()) status_ = kEndOfStream;
      return;
    }
    switch (errno) {
      case EFBIG:
      case EINVAL:
      case EOVERFLOW:
        status_ = kSeekError;
        break;
      default:
        status_ = kUnknownIOError;
        break;
    }
  }

  uint64_t position() const override { return (::ftell(file_)); }

  uint64_t size() override {
    if (status_ != kOk && status_ != kEndOfStream) return 0;
    struct stat st;
    if (::fstat(fileno(file_), &st) != 0) {
      status_ = kUnknownIOError;
      return 0;
    }
    return st.st_size;
  }

  bool isOpen() const override {
    return status_ == kOk || status_ == kEndOfStream;
  }

  void close() override {
    if (status_ != kOk && status_ != kEndOfStream) return;
    if (::fclose(file_) == 0) {
      status_ = kClosed;
      file_ = nullptr;
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
  mutable Status status_;
};

}  // namespace roo_io