#include "roo_io/fs/esp32/spiffs.h"

#if defined(ROO_TESTING)

#include "roo_testing/devices/microcontroller/esp32/fake_esp32.h"

#endif

#if (defined(ESP32) || defined(ROO_TESTING))

#include "esp32-hal.h"
#include "esp_spiffs.h"
#include "roo_io/fs/posix/posix_mount.h"

#if !defined(MLOG_roo_io)
#define MLOG_roo_io 0
#endif

namespace roo_io {
namespace esp32 {

SpiffsFs::SpiffsFs() : SpiffsFs("/spiffs", nullptr) {}

SpiffsFs::SpiffsFs(const char* mount_point, const char* partition_label)
    : mount_point_(mount_point),
      has_partition_label_(partition_label != nullptr),
      partition_label_(partition_label == nullptr ? "" : partition_label) {}

const char* SpiffsFs::mount_point() const { return mount_point_.c_str(); }

void SpiffsFs::set_mount_point(const char* mount_point) {
  mount_point_ = mount_point;
}

const char* SpiffsFs::partition_label() const {
  return has_partition_label_ ? partition_label_.c_str() : nullptr;
}

void SpiffsFs::set_partition_label(const char* partition_label) {
  has_partition_label_ = (partition_label != nullptr);
  partition_label_ = partition_label;
}

uint8_t SpiffsFs::max_files() const { return max_files_; }

void SpiffsFs::set_max_files(uint8_t max_files) { max_files_ = max_files; }

bool SpiffsFs::format_if_empty() const { return format_if_empty_; }

void SpiffsFs::set_format_if_empty(bool format_if_empty) {
  format_if_empty_ = format_if_empty;
}

MountImpl::MountResult SpiffsFs::mountImpl(std::function<void()> unmount_fn) {
  MLOG(roo_io) << "Mounting SPIFFS";
  std::string mount_base_path;
#if defined(ROO_TESTING)
  mount_base_path = FakeEsp32().fs_root();
#endif
  mount_base_path.append(mount_point_);

  esp_vfs_spiffs_conf_t conf = {.base_path = mount_base_path.c_str(),
                                .partition_label = partition_label(),
                                .max_files = max_files_,
                                .format_if_mount_failed = false};

  esp_err_t ret = esp_vfs_spiffs_register(&conf);
  if (ret == ESP_FAIL && format_if_empty_) {
    if (format()) {
      ret = esp_vfs_spiffs_register(&conf);
    }
  }
  if (ret != ESP_OK) {
    LOG(ERROR) << "Mounting SPIFFS failed! Error: " << esp_err_to_name(ret);
    return MountImpl::MountError(kMountError);
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mount_base_path.c_str(), false, unmount_fn)));
}

void SpiffsFs::unmountImpl() {
  MLOG(roo_io) << "Unmounting SPIFFS";
  esp_err_t err = esp_vfs_spiffs_unregister(partition_label());
  if (err) {
    LOG(ERROR) << "Unmounting SPIFFS failed! Error: " << esp_err_to_name(err);
    return;
  }
  mounted_partition_label_.clear();
}

Status SpiffsFs::format() {
  disableCore0WDT();
  esp_err_t err = esp_spiffs_format(partition_label_.c_str());
  enableCore0WDT();
  if (err) {
    LOG(ERROR) << "Formatting SpiffsFs failed! Error: " << esp_err_to_name(err);
    return kUnknownIOError;
  }
  return kOk;
}

Filesystem::MediaPresence SpiffsFs::checkMediaPresence() {
  if (isMounted()) return Filesystem::kMediaPresent;
  Mount m = mount();
  if (m.ok()) return Filesystem::kMediaPresent;
  return Filesystem::kMediaAbsent;
}

SpiffsFs SPIFFS;

}  // namespace esp32
}  // namespace roo_io

#endif
