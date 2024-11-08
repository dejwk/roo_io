#pragma once

#include <FS.h>

#include "roo_io/stream/random_access_input_stream.h"

namespace roo_io {

class ArduinoFileInputStream : public RandomAccessInputStream {
 public:
  ArduinoFileInputStream(Status error) : file_(), status_(error) {}

  ArduinoFileInputStream(fs::File file)
      : file_(std::move(file)), status_(file_ ? kOk : kClosed) {}

  int read(uint8_t* buf, unsigned int count) override {
    int result = file_.read(buf, count);
    if (result == 0 && status_ == roo_io::kOk) {
      status_ = kEndOfStream;
    } else if (result < 0 && status_ == roo_io::kOk) {
      status_ = file_.operator bool() ? roo_io::kReadError : roo_io::kClosed;
    }
    return result;
  }

  bool seek(uint64_t offset) override { return file_.seek(offset); }

  bool skip(uint64_t count) override { return file_.seek(count, SeekCur); }

  uint64_t position() const override { return file_.position(); }

  uint64_t size() const override { return file_.size(); }

  bool isOpen() const override { return file_.operator bool(); }

  void close() override {
    file_.close();
    status_ = kClosed;
  }

  Status status() const override { return status_; }

 private:
  fs::File file_;
  Status status_;
};

}  // namespace roo_io