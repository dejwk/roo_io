#include "roo_io/fs/esp32/spiffs.h"

#if defined(ROO_TESTING)

#include "roo_testing/microcontrollers/esp32/fake_esp32.h"

#endif

#if (defined(ESP_PLATFORM) || defined(ROO_TESTING)) && \
    __has_include("esp_spiffs.h")

#include "esp_spiffs.h"
#include "esp_task_wdt.h"
#include "roo_io/fs/posix/posix_mount.h"

#if !defined(MLOG_roo_io_fs)
#define MLOG_roo_io_fs 0
#endif

namespace roo_io {

SpiffsFs::SpiffsFs()
    : mount_point_("/spiffs"),
      max_open_files_(10),
      format_if_mount_failed_(false),
      has_partition_label_(false),
      partition_label_("") {}

const char* SpiffsFs::mountPoint() const { return mount_point_.c_str(); }

void SpiffsFs::setMountPoint(const char* mount_point) {
  mount_point_ = mount_point;
}

const char* SpiffsFs::partitionLabel() const {
  return has_partition_label_ ? partition_label_.c_str() : nullptr;
}

void SpiffsFs::setPartitionLabel(const char* partition_label) {
  has_partition_label_ = (partition_label != nullptr);
  partition_label_ = partition_label;
}

uint8_t SpiffsFs::maxOpenFiles() const { return max_open_files_; }

void SpiffsFs::setMaxOpenFiles(uint8_t max_open_files) {
  max_open_files_ = max_open_files;
}

bool SpiffsFs::formatIfMountFailed() const { return format_if_mount_failed_; }

void SpiffsFs::setFormatIfMountFailed(bool format_if_mount_failed) {
  format_if_mount_failed_ = format_if_mount_failed;
}

MountImpl::MountResult SpiffsFs::mountImpl(std::function<void()> unmount_fn) {
  MLOG(roo_io_fs) << "Mounting SPIFFS";
  std::string mount_base_path;
#if defined(ROO_TESTING)
  mount_base_path = FakeEsp32().fs_root();
#endif
  mount_base_path.append(mount_point_);

  esp_vfs_spiffs_conf_t conf = {
      .base_path = mount_base_path.c_str(),
      .partition_label = partitionLabel(),
      .max_files = max_open_files_,
      .format_if_mount_failed = format_if_mount_failed_};

  esp_err_t ret = esp_vfs_spiffs_register(&conf);
  if (ret == ESP_FAIL && format_if_mount_failed_) {
    if (format()) {
      ret = esp_vfs_spiffs_register(&conf);
    }
  }
  if (ret != ESP_OK) {
    LOG(ERROR) << "Mounting SPIFFS failed! Error: " << esp_err_to_name(ret);
    if (ret == ESP_ERR_NO_MEM) {
      return MountImpl::MountError(kOutOfMemory);
    } else {
      return MountImpl::MountError(kGenericMountError);
    }
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mount_base_path.c_str(), false, unmount_fn)));
}

void SpiffsFs::unmountImpl() {
  MLOG(roo_io_fs) << "Unmounting SPIFFS";
  esp_err_t err = esp_vfs_spiffs_unregister(partitionLabel());
  if (err) {
    LOG(ERROR) << "Unmounting SPIFFS failed! Error: " << esp_err_to_name(err);
    return;
  }
  mounted_partition_label_.clear();
}

Status SpiffsFs::format() {
#ifndef ROO_TESTING
  esp_task_wdt_delete(xTaskGetIdleTaskHandleForCore(0));
#endif
  esp_err_t err = esp_spiffs_format(partition_label_.c_str());
#ifndef ROO_TESTING
  esp_task_wdt_add(xTaskGetIdleTaskHandleForCore(0));
#endif
  if (err) {
    LOG(ERROR) << "Formatting SpiffsFs failed! Error: " << esp_err_to_name(err);
    return kUnknownIOError;
  }
  return kOk;
}

Filesystem::MediaPresence SpiffsFs::checkMediaPresence() {
  if (isMounted()) return Filesystem::kMediaPresent;
  // Mount m = mount();
  // if (m.ok()) return Filesystem::kMediaPresent;
  return Filesystem::kMediaPresenceUnknown;
}

SpiffsFs CreateSpiffsFs() { return SpiffsFs(); }

SpiffsFs SPIFFS = CreateSpiffsFs();

}  // namespace roo_io

#endif
