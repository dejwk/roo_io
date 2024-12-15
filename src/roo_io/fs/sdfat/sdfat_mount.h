#pragma once

#include <SdFat.h>

#include <functional>

#include "roo_io/fs/filesystem.h"

namespace roo_io {

class SdFatMountImpl : public MountImpl {
 public:
  SdFatMountImpl(SdFs& fs, bool read_only, std::function<void()> unmount_fn);

  bool isReadOnly() const override;

  Stat stat(const char* path) const override;

  Status remove(const char* path) override;

  Status rename(const char* pathFrom, const char* pathTo) override;

  Status mkdir(const char* path) override;

  Status rmdir(const char* path) override;

  std::unique_ptr<DirectoryImpl> opendir(const char* path) override;

  std::unique_ptr<MultipassInputStream> fopen(const char* path) override;

  std::unique_ptr<OutputStream> fopenForWrite(
      const char* path, FileUpdatePolicy update_policy) override;

  bool active() const { return active_; }

  void deactivate() { active_ = false; }

 private:
  SdFs& fs_;
  bool active_;
  bool read_only_;
};

}  // namespace roo_io