#pragma once

#include <FS.h>

#include <functional>

#include "roo_io/fs/arduino/arduino_file.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

class ArduinoMountImpl : public MountImpl {
 public:
  ArduinoMountImpl(FS& fs, bool read_only, std::function<void()> unmount_fn);

  bool isReadOnly() const override;

  Stat stat(const char* path) const override;

  Status remove(const char* path) override;

  Status rename(const char* pathFrom, const char* pathTo) override;

  Status mkdir(const char* path) override;

  Status rmdir(const char* path) override;

  std::unique_ptr<FileImpl> openForReading(const char* path) override;

  std::unique_ptr<FileImpl> createOrReplace(const char* path) override;

  std::unique_ptr<FileImpl> openForAppend(const char* path) override;

 private:
  FS& fs_;
  bool read_only_;
};

}  // namespace roo_io