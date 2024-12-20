#pragma once

#include <SdFat.h>

#include "roo_io/core/output_stream.h"

namespace roo_io {

class SdFatFileOutputStream : public OutputStream {
 public:
  SdFatFileOutputStream(Status error) : file_(), status_(error) {}

  SdFatFileOutputStream(FsFile file)
      : file_(std::move(file)), status_(file_ ? kOk : kClosed) {}

  ~SdFatFileOutputStream() { file_.close(); }

  size_t write(const byte* buf, size_t count) override {
    if (status_ != kOk) return 0;
    size_t result = file_.write(buf, count);
    if (result < count) {
      status_ = roo_io::kWriteError;
    }
    return result;
  }

  void flush() override {
    if (status_ == kClosed) return;
    file_.flush();
    if (!file_) {
      status_ = kWriteError;
    }
  }

  void close() override {
    if (status_ == kClosed) return;
    file_.close();
    if (status_ == kOk) {
      status_ = kClosed;
    }
  }

  Status status() const override { return status_; }

 private:
  FsFile file_;
  Status status_;
};

}  // namespace roo_io