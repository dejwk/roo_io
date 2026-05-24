#pragma once

#if (defined ESP_PLATFORM || defined ROO_TESTING) && \
    __has_include("esp_spiffs.h")

#include "roo_io/fs/filesystem.h"

namespace roo_io {

/// ESP32 SPIFFS filesystem wrapper.
class SpiffsFs : public Filesystem {
 public:
  /// Returns the mount point used for future mounts.
  const char* mountPoint() const;
  /// Sets the mount point used for future mounts.
  void setMountPoint(const char* mount_point);

  /// Returns the partition label used for future mounts, if configured.
  const char* partitionLabel() const;
  /// Sets the partition label used for future mounts.
  void setPartitionLabel(const char* partition_label);

  /// Returns the maximum number of simultaneously open files.
  uint8_t maxOpenFiles() const;
  /// Sets the maximum number of simultaneously open files.
  void setMaxOpenFiles(uint8_t max_files);

  /// Returns whether the filesystem will be formatted after mount failure.
  bool formatIfMountFailed() const;
  /// Sets whether the filesystem will be formatted after mount failure.
  void setFormatIfMountFailed(bool format_if_mount_failed);

  /// Probes whether the SPIFFS partition is present.
  MediaPresence checkMediaPresence() override;

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

  Status format();

 private:
  friend SpiffsFs CreateSpiffsFs();

  SpiffsFs();

  std::string mount_point_;
  uint8_t max_open_files_;
  bool format_if_mount_failed_;

  bool has_partition_label_;
  std::string partition_label_;
  std::string mounted_partition_label_;
};

/// Global SPIFFS filesystem instance.
extern SpiffsFs SPIFFS;

}  // namespace roo_io

#endif  // ESP_PLATFORM