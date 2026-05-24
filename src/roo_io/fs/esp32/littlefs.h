#pragma once

#if (defined ESP_PLATFORM || defined ROO_TESTING) && \
    __has_include("esp_littlefs.h")

#include "roo_io/fs/filesystem.h"

namespace roo_io {

/// ESP32 LittleFS filesystem wrapper.
class LittlefsFs : public Filesystem {
 public:
  /// Returns the mount point used for future mounts.
  const char* mountPoint() const;
  /// Sets the mount point used for future mounts.
  void setMountPoint(const char* mount_point);

  /// Returns the partition label used for future mounts, if configured.
  const char* partitionLabel() const;
  /// Sets the partition label used for future mounts.
  void setPartitionLabel(const char* partition_label);

  /// Returns whether the filesystem will be formatted after mount failure.
  bool formatIfMountFailed() const;
  /// Sets whether the filesystem will be formatted after mount failure.
  void setFormatIfMountFailed(bool format_if_mount_failed);

  /// Probes whether the LittleFS partition is present.
  MediaPresence checkMediaPresence() override;

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

  Status format();

 private:
  friend LittlefsFs CreateLittlefsFs();

  LittlefsFs();

  std::string mount_point_;
  bool format_if_mount_failed_;

  bool has_partition_label_;
  std::string partition_label_;
  std::string mounted_partition_label_;
};

/// Global LittleFS filesystem instance.
extern LittlefsFs LITTLEFS;

}  // namespace roo_io

#endif  // ESP_PLATFORM