#pragma once

#if (defined ESP_PLATFORM || defined ROO_TESTING) && \
    __has_include("esp_spiffs.h")

#include "roo_io/fs/filesystem.h"

namespace roo_io {

class SpiffsFs : public Filesystem {
 public:
  const char* mountPoint() const;
  void setMountPoint(const char* mount_point);

  const char* partitionLabel() const;
  void setPartitionLabel(const char* partition_label);

  uint8_t maxOpenFiles() const;
  void setMaxOpenFiles(uint8_t max_files);

  bool formatIfMountFailed() const;
  void setFormatIfMountFailed(bool format_if_mount_failed);

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

extern SpiffsFs SPIFFS;

}  // namespace roo_io

#endif  // ESP_PLATFORM