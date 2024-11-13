#pragma once

#include "roo_io/fs/filesystem.h"

namespace roo_io {
namespace esp32 {

class SpiffsFs : public Filesystem {
 public:
  SpiffsFs();
  SpiffsFs(const char* mount_point, const char* partition_label = nullptr);

  const char* mount_point() const;
  void set_mount_point(const char* mount_point);

  const char* partition_label() const;
  void set_partition_label(const char* partition_label);

  uint8_t max_files() const;
  void set_max_files(uint8_t max_files);

  bool format_if_empty() const;
  void set_format_if_empty(bool format_if_empty);

  MediaPresence checkMediaPresence() override;

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

  Status format();

 private:
  std::string mount_point_;
  uint8_t max_files_;
  bool format_if_empty_;

  uint32_t spi_frequency_;

  bool has_partition_label_;
  std::string partition_label_;
  std::string mounted_partition_label_;
};

extern SpiffsFs SPIFFS;

}  // namespace esp32
}  // namespace roo_io