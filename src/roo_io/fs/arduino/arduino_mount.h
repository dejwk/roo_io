#pragma once

#include <FS.h>

#include <functional>

#include "roo_io/fs/arduino/arduino_file.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

class ArduinoMountImpl : public MountImpl {
 public:
  ArduinoMountImpl(FS& fs, std::function<void()> unmount_fn);

  bool exists(const char* path) const override;

  Status remove(const char* path) override;

  bool rename(const char* pathFrom, const char* pathTo) override;

  Status mkdir(const char* path) override;

  bool rmdir(const char* path) override;

  std::unique_ptr<FileImpl> openForReading(const char* path) override;

  std::unique_ptr<FileImpl> createOrReplace(const char* path) override;

  std::unique_ptr<FileImpl> openForAppend(const char* path) override;

 private:
  FS& fs_;
};

}  // namespace roo_io