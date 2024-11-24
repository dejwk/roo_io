#pragma once

#include <FS.h>

#include "roo_io/stream/output_stream.h"

namespace roo_io {

class ArduinoFileOutputStream : public OutputStream {
 public:
  ArduinoFileOutputStream(Status error)
      : file_(), status_(error) {}

  ArduinoFileOutputStream(fs::File file)
      : file_(std::move(file)), status_(file_ ? kOk : kClosed) {}

  int write(const uint8_t* buf, unsigned int count) override {
    int result = file_.write(buf, count);
    if (result == 0 && status_ == roo_io::kOk) {
      status_ = roo_io::kReadError;
    }
    return result;
  }

  bool isOpen() const override { return file_.operator bool(); }

  void close() override {
    file_.close();
    status_ = kClosed;
  }

  void flush() override {
    file_.flush();
  }

  Status status() const override { return status_; }

 private:
  fs::File file_;
  Status status_;
};

}  // namespace roo_io