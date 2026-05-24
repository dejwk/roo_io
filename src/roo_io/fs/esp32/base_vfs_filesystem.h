#pragma once

#if (defined ESP_PLATFORM || defined ROO_TESTING)

#include "roo_io/fs/filesystem.h"

namespace roo_io {

/// Common base class for ESP32 filesystems mounted through the VFS layer.
class BaseEsp32VfsFilesystem : public Filesystem {
 public:
  /// Returns the SPI or bus frequency used for future mounts.
  uint32_t frequency() const { return frequency_; }

  /// Sets the SPI or bus frequency used for future mounts.
  void setFrequency(uint32_t freq) { frequency_ = freq; }

  /// Returns the mount point path used for future mounts.
  const char* mountPoint() const { return mount_point_.c_str(); }

  /// Sets the mount point path used for future mounts.
  void setMountPoint(const char* mount_point) { mount_point_ = mount_point; }

  /// Returns the maximum number of simultaneously open files.
  uint8_t maxOpenFiles() const { return max_open_files_; }

  /// Sets the maximum number of simultaneously open files.
  void setMaxOpenFiles(uint8_t max_open_files) {
    max_open_files_ = max_open_files;
  }

  /// Returns whether the filesystem should be reformatted after mount failure.
  bool formatIfMountFailed() const { return format_if_mount_failed_; }

  /// Sets whether the filesystem should be reformatted after mount failure.
  void setFormatIfMountFailed(bool format_if_mount_failed) {
    format_if_mount_failed_ = format_if_mount_failed;
  }

  /// Returns whether new mounts are forced read-only.
  bool readOnly() const { return read_only_; }

  /// Sets whether new mounts are forced read-only.
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
