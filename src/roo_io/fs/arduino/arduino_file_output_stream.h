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

  void flush() override {
    file_.flush();
  }

  Status status() const override { return status_; }

 private:
  fs::File file_;
  Status status_;
};

}  // namespace roo_io