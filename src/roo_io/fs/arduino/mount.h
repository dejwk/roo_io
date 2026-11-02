#pragma once

#ifdef ARDUINO

#include <FS.h>

#include <functional>

#include "roo_io/core/multipass_input_stream.h"
#include "roo_io/fs/arduino/directory.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

class ArduinoMountImpl : public MountImpl {
 public:
  /// Borrows @p fs for the mount lifetime with the given read-only policy.
  ArduinoMountImpl(FS& fs, bool read_only, std::function<void()> unmount_fn);

  bool isReadOnly() const override;

  Stat stat(const char* path) const override;

  Status remove(const char* path) override;

  Status rename(const char* pathFrom, const char* pathTo) override;

  Status mkdir(const char* path) override;

  Status rmdir(const char* path) override;

  std::unique_ptr<DirectoryImpl> opendir(std::shared_ptr<MountImpl> mount,
                                         const char* path) override;

  std::unique_ptr<MultipassInputStream> fopen(std::shared_ptr<MountImpl> mount,
                                              const char* path) override;

  std::unique_ptr<OutputStream> fopenForWrite(
      std::shared_ptr<MountImpl> mount, const char* path,
      FileUpdatePolicy update_policy) override;

  bool active() const override { return active_; }

  void deactivate() override;

 protected:
  /// Deletes a validated file or directory through the native FS API.
  /// Overrides adapt backend deletion behavior for remove, rmdir and rename
  /// replacement. Return false if deletion or the required adaptation fails.
  virtual bool removeEntry(const char* path, bool directory);

  /// Returns the borrowed backend for platform-specific mount operations.
  FS& filesystem() const { return fs_; }

 private:
  FS& fs_;
  bool active_;
  bool read_only_;
};

}  // namespace roo_io

#endif  // ARDUINO
