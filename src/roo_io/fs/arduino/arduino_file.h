#pragma once

#include <memory>

#include "FS.h"
#include "roo_io/fs/arduino/arduino_file_input_stream.h"
#include "roo_io/fs/arduino/arduino_file_output_stream.h"
#include "roo_io/fs/file.h"
#include "roo_io/stream/output_stream.h"
#include "roo_io/stream/null_input_stream.h"
#include "roo_io/stream/random_access_input_stream.h"

namespace roo_io {

class ArduinoFileImpl : public FileImpl {
 public:
  ArduinoFileImpl(fs::File file, Status status)
      : file_(std::move(file)), status_(status) {}

  bool isOpen() const override { return file_.operator bool(); }

  bool close() override {
    file_.close();
    if (status_ = kOk) status_ = kClosed;
    return true;
  }

  Status status() const override { return status_; }

  bool isDirectory() const override { return file_.isDirectory(); }

  uint64_t size() const override { return file_.size(); }

  const char* path() const override { return file_.path(); }

  const char* name() const override { return file_.name(); }

  void rewindDirectory() override { file_.rewindDirectory(); }

  std::unique_ptr<FileImpl> openNextFile() override {
    fs::File next = file_.openNextFile();
    return std::unique_ptr<ArduinoFileImpl>(
        new ArduinoFileImpl(next, next.operator bool() ? kOk : kClosed));
  }

  std::unique_ptr<RandomAccessInputStream> asInputStream() && override {
    return std::unique_ptr<RandomAccessInputStream>(
        status_ == kOk
            ? (RandomAccessInputStream*)new ArduinoFileInputStream(
                  std::move(file_))
            : (RandomAccessInputStream*)new NullInputStream(status_));
  }

  std::unique_ptr<OutputStream> asOutputStream() && override {
    return std::unique_ptr<OutputStream>(
        status_ == kOk ? (OutputStream*)new ArduinoFileOutputStream(
                             std::move(file_))
                       : (OutputStream*)new NullOutputStream(status_));
  }

 private:
  mutable fs::File file_;
  Status status_;
};

}  // namespace roo_io