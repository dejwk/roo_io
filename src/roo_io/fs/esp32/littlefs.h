#pragma once

#if (defined ESP32 || defined ROO_TESTING)

#include "roo_io/fs/filesystem.h"

namespace roo_io {

class LittlefsFs : public Filesystem {
 public:
  const char* mountPoint() const;
  void setMountPoint(const char* mount_point);

  const char* partitionLabel() const;
  void setPartitionLabel(const char* partition_label);

  bool formatIfMountFailed() const;
  void setFormatIfMountFailed(bool format_if_mount_failed);

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

extern LittlefsFs LITTLEFS;

}  // namespace roo_io

#endif  // ESP32