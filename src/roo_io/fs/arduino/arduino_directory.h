#pragma once

#include <memory>

#include "Arduino.h"
#include "FS.h"
#include "roo_io/fs/directory.h"

namespace roo_io {

class ArduinoDirectoryImpl : public DirectoryImpl {
 public:
  ArduinoDirectoryImpl(fs::File file, Status status)
      : file_(std::move(file)), status_(status) {}

  bool isOpen() const override { return file_.operator bool(); }

  bool close() override {
    entry_.close();
    file_.close();
    if (status_ = kOk) status_ = kClosed;
    return true;
  }

  const char* path() const override { return file_.path(); }

  const char* name() const override { return file_.name(); }

  Status status() const override { return status_; }

  void rewind() override {
    if (status_ != kOk) return;
    file_.rewindDirectory();
    if (!file_) status_ = kClosed;
    next_ = "";
  }

  Entry read() override {
    if (status_ != kOk) return Entry();
    if (!file_) {
      status_ = kClosed;
      return Entry();
    }
    entry_ = file_.openNextFile();
    if (!entry_) return Entry();
    return DirectoryImpl::CreateEntry(
        entry_.path(), strlen(entry_.path()) - strlen(entry_.name()),
        entry_.isDirectory());
  }

 private:
  mutable fs::File file_;
  fs::File entry_;
  String next_;
  Status status_;
};

}  // namespace roo_io