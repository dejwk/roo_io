#pragma once

#include "roo_io/fs/posix/config.h"

#if ROO_IO_FS_SUPPORT_POSIX

#include <memory>

#include "roo_io/fs/filesystem.h"

namespace roo_io {

/// POSIX-backed mount implementation rooted at a host mount point.
class PosixMountImpl : public MountImpl {
 public:
  /// Creates a mount implementation rooted at `mount_point`.
  PosixMountImpl(const char* mount_point, bool read_only,
                 std::function<void()> unmount_fn);

  /// Returns whether this mount is configured read-only.
  bool isReadOnly() const override;

  /// Stats the file or directory at `path`.
  Stat stat(const char* path) const override;

  /// Removes the file at `path`.
  Status remove(const char* path) override;

  /// Renames or moves an entry.
  Status rename(const char* pathFrom, const char* pathTo) override;

  /// Creates the directory at `path`.
  Status mkdir(const char* path) override;

  /// Removes the empty directory at `path`.
  Status rmdir(const char* path) override;

  /// Opens the directory at `path`.
  std::unique_ptr<DirectoryImpl> opendir(std::shared_ptr<MountImpl> mount,
                                         const char* path) override;

  /// Opens the file at `path` for reading.
  std::unique_ptr<MultipassInputStream> fopen(std::shared_ptr<MountImpl> mount,
                                              const char* path) override;

  /// Opens the file at `path` for writing using `update_policy`.
  std::unique_ptr<OutputStream> fopenForWrite(
      std::shared_ptr<MountImpl> mount, const char* path,
      FileUpdatePolicy update_policy) override;

  /// Returns whether the mount implementation is still active.
  bool active() const override { return mount_point_ != nullptr; }

  /// Deactivates the mount implementation.
  void deactivate() override;

 private:
  std::unique_ptr<char[]> mount_point_;
  bool active_;
  bool read_only_;
};

}  // namespace roo_io

#endif  // ROO_IO_FS_SUPPORT_POSIX
