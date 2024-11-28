#pragma once

#include <SdFat.h>

#include "roo_io/stream/multipass_input_stream.h"

namespace roo_io {

class SdFatFileInputStream : public MultipassInputStream {
 public:
  SdFatFileInputStream(Status error) : file_(), status_(error) {}

  SdFatFileInputStream(FsFile file)
      : file_(std::move(file)), status_(file_ ? kOk : kClosed) {}

  size_t read(byte* buf, size_t count) override {
    if (status_ != kOk) return 0;
    int result = file_.read(buf, count);
    if (result > 0) return result;
    if (result == 0) {
      status_ = kEndOfStream;
    } else {
      status_ = file_.getError() != 0 ? roo_io::kReadError : roo_io::kClosed;
    }
    return 0;
  }

  bool seek(uint64_t offset) override { return file_.seek(offset); }

  bool skip(uint64_t count) override { return file_.seekCur(count); }

  uint64_t position() const override { return file_.position(); }

  uint64_t size() const override { return file_.size(); }

  bool isOpen() const override { return file_.operator bool(); }

  void close() override {
    file_.close();
    status_ = kClosed;
  }

  Status status() const override { return status_; }

 private:
  mutable FsFile file_;
  Status status_;
};

}  // namespace roo_io