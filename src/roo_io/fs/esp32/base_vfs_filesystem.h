#pragma once

#if (defined ESP_PLATFORM || defined ROO_TESTING)

#include "roo_io/fs/filesystem.h"

namespace roo_io {

// Common base class for ESP32 filesystems using ESP-IDF VFS FAT APIs.
// Allows setting common parameters like frequency, mount point, max open
// files, format-if-mount-failed, read-only mode.
class BaseEsp32VfsFilesystem : public Filesystem {
 public:
  uint32_t frequency() const { return frequency_; }

  void setFrequency(uint32_t freq) { frequency_ = freq; }

  const char* mountPoint() const { return mount_point_.c_str(); }

  void setMountPoint(const char* mount_point) { mount_point_ = mount_point; }

  uint8_t maxOpenFiles() const { return max_open_files_; }

  void setMaxOpenFiles(uint8_t max_open_files) {
    max_open_files_ = max_open_files;
  }

  bool formatIfMountFailed() const { return format_if_mount_failed_; }

  void setFormatIfMountFailed(bool format_if_mount_failed) {
    format_if_mount_failed_ = format_if_mount_failed;
  }

  bool readOnly() const { return read_only_; }

  void setReadOnly(bool read_only) { read_only_ = read_only; }

 protected:
  BaseEsp32VfsFilesystem(uint32_t frequency, const char* mount_point)
      : frequency_(frequency),
        mount_point_(mount_point),
        max_open_files_(5),
        format_if_mount_failed_(false),
        read_only_(false) {}

 private:
  uint32_t frequency_;
  std::string mount_point_;
  uint8_t max_open_files_;
  bool format_if_mount_failed_;
  bool read_only_;
};

}  // namespace roo_io

#endif  // ESP_PLATFORM
