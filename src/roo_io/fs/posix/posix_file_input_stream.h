#pragma once

#include <stdio.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include "roo_io/fs/filesystem.h"
#include "roo_io/stream/multipass_input_stream.h"

namespace roo_io {

class PosixFileInputStream : public MultipassInputStream {
 public:
  PosixFileInputStream(Status error) : file_(nullptr), size_(-1), status_(error) {}

  PosixFileInputStream(FILE* file)
      : file_(file), size_(-1), status_(file_ != nullptr ? kOk : kClosed) {}

  int read(byte* buf, unsigned int count) override {
    if (status_ != kOk && status_ != kEndOfStream) return -1;
    int result = fread(buf, 1, count, file_);
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

  bool seek(uint64_t offset) override {
    if (status_ != kOk && status_ != kEndOfStream) return false;
    if (::fseek(file_, offset, SEEK_SET) == 0) {
      return true;
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
    return false;
  }

  bool skip(uint64_t count) override {
    if (status_ != kOk && status_ != kEndOfStream) return false;
    if (::fseek(file_, count, SEEK_CUR) == 0) {
      return true;
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
    return false;
  }

  uint64_t position() const override {
    if (status_ != kOk && status_ != kEndOfStream) return false;
    return (::ftell(file_));
  }

  uint64_t size() const override {
    if (status_ != kOk && status_ != kEndOfStream) return 0;
    if (size_ < 0) {
      struct stat st;
      if (::fstat(fileno(file_), &st) != 0) {
        status_ = kUnknownIOError;
        return 0;
      }
      size_ = st.st_size;
    }
    return size_;
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