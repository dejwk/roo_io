#pragma once

#include <FS.h>

#include <functional>

#include "roo_io/fs/arduino/arduino_file.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

class ArduinoMountImpl : public MountImpl {
 public:
  ArduinoMountImpl(FS& fs, std::function<void()> unmount_fn)
      : MountImpl(unmount_fn), fs_(fs) {}

  bool exists(const char* path) const override { return fs_.exists(path); }

  bool remove(const char* path) override { return fs_.remove(path); }

  bool rename(const char* pathFrom, const char* pathTo) override {
    return fs_.rename(pathFrom, pathTo);
  }

  bool mkdir(const char* path) override { return fs_.mkdir(path); }

  bool rmdir(const char* path) override { return fs_.rmdir(path); }

  std::unique_ptr<FileImpl> openForReading(const char* path) override {
    fs::File f = fs_.open(path, "r");
    roo_io::Status status = roo_io::kOk;
    if (!f) {
      if (!fs_.exists(path)) {
        status = roo_io::kNotFound;
      } else {
        status = roo_io::kOpenError;
      }
    }
    return std::unique_ptr<FileImpl>(new ArduinoFileImpl(std::move(f), status));
  }

  std::unique_ptr<FileImpl> createOrReplace(const char* path) {
    fs::File f = fs_.open(path, "w", true);
    roo_io::Status status = roo_io::kOk;
    if (!f) {
      status = roo_io::kOpenError;
    }
    return std::unique_ptr<FileImpl>(new ArduinoFileImpl(std::move(f), status));
  }

  std::unique_ptr<FileImpl> openForAppend(const char* path) {
    fs::File f = fs_.open(path, "a", false);
    roo_io::Status status = roo_io::kOk;
    if (!f) {
      if (!fs_.exists(path)) {
        status = roo_io::kNotFound;
      } else {
        status = roo_io::kOpenError;
      }
    }
    return std::unique_ptr<FileImpl>(new ArduinoFileImpl(std::move(f), status));
  }

 private:
  FS& fs_;
};

}  // namespace roo_io