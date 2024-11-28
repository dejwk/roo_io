#pragma once

#include <SdFat.h>

#include "roo_io/stream/output_stream.h"

namespace roo_io {

class SdFatFileOutputStream : public OutputStream {
 public:
  SdFatFileOutputStream(Status error)
      : file_(), status_(error) {}

  SdFatFileOutputStream(FsFile file)
      : file_(std::move(file)), status_(file_ ? kOk : kClosed) {}

  size_t write(const byte* buf, size_t count) override {
    if (status_ != kOk) return 0;
    size_t result = file_.write(buf, count);
    if (result < count) {
      status_ = roo_io::kWriteError;
    }
    return result;
  }

  bool isOpen() const override { return file_.operator bool(); }

  void close() override {
    file_.close();
    status_ = kClosed;
  }

  Status status() const override { return status_; }

 private:
  FsFile file_;
  Status status_;
};

}  // namespace roo_io