#include "roo_io/fs/esp32/littlefs.h"

#if defined(ROO_TESTING)

#include "roo_testing/microcontrollers/esp32/fake_esp32.h"

#endif

// See https://github.com/joltwallet/esp_littlefs?tab=readme-ov-file#how-to-use

#if (defined(ESP_PLATFORM) || defined(ROO_TESTING)) && \
    __has_include("esp_littlefs.h")

#include "esp_littlefs.h"
#include "esp_task_wdt.h"
#include "roo_io/fs/posix/posix_mount.h"

#if !defined(MLOG_roo_io_fs)
#define MLOG_roo_io_fs 0
#endif

namespace roo_io {

LittlefsFs::LittlefsFs()
    : mount_point_("/littlefs"),
      format_if_mount_failed_(false),
      has_partition_label_(false),
      partition_label_("") {}

const char* LittlefsFs::mountPoint() const { return mount_point_.c_str(); }

void LittlefsFs::setMountPoint(const char* mount_point) {
  mount_point_ = mount_point;
}

const char* LittlefsFs::partitionLabel() const {
  return has_partition_label_ ? partition_label_.c_str() : nullptr;
}

void LittlefsFs::setPartitionLabel(const char* partition_label) {
  has_partition_label_ = (partition_label != nullptr);
  partition_label_ = partition_label;
}

bool LittlefsFs::formatIfMountFailed() const { return format_if_mount_failed_; }

void LittlefsFs::setFormatIfMountFailed(bool format_if_mount_failed) {
  format_if_mount_failed_ = format_if_mount_failed;
}

MountImpl::MountResult LittlefsFs::mountImpl(std::function<void()> unmount_fn) {
  MLOG(roo_io_fs) << "Mounting LITTLEFS";
  std::string mount_base_path;
#if defined(ROO_TESTING)
  mount_base_path = FakeEsp32().fs_root();
#endif
  mount_base_path.append(mount_point_);

  esp_vfs_littlefs_conf_t conf = {
      .base_path = mount_base_path.c_str(),
      .partition_label = partitionLabel(),
      .format_if_mount_failed = format_if_mount_failed_};

  esp_err_t ret = esp_vfs_littlefs_register(&conf);
  if (ret == ESP_FAIL && format_if_mount_failed_) {
    if (format()) {
      ret = esp_vfs_littlefs_register(&conf);
    }
  }
  if (ret != ESP_OK) {
    LOG(ERROR) << "Mounting LITTLEFS failed! Error: " << esp_err_to_name(ret);
    if (ret == ESP_ERR_NO_MEM) {
      return MountImpl::MountError(kOutOfMemory);
    } else {
      return MountImpl::MountError(kGenericMountError);
    }
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mount_base_path.c_str(), false, unmount_fn)));
}

void LittlefsFs::unmountImpl() {
  MLOG(roo_io_fs) << "Unmounting LITTLEFS";
  esp_err_t err = esp_vfs_littlefs_unregister(partitionLabel());
  if (err) {
    LOG(ERROR) << "Unmounting LITTLEFS failed! Error: " << esp_err_to_name(err);
    return;
  }
  mounted_partition_label_.clear();
}

Status LittlefsFs::format() {
#ifndef ROO_TESTING
  esp_task_wdt_delete(xTaskGetIdleTaskHandleForCore(0));
#endif
  esp_err_t err = esp_littlefs_format(partition_label_.c_str());
#ifndef ROO_TESTING
  esp_task_wdt_add(xTaskGetIdleTaskHandleForCore(0));
#endif
  if (err) {
    LOG(ERROR) << "Formatting LITTLEFS failed! Error: " << esp_err_to_name(err);
    return kUnknownIOError;
  }
  return kOk;
}

Filesystem::MediaPresence LittlefsFs::checkMediaPresence() {
  if (isMounted()) return Filesystem::kMediaPresent;
  // Mount m = mount();
  // if (m.ok()) return Filesystem::kMediaPresent;
  return Filesystem::kMediaPresenceUnknown;
}

LittlefsFs CreateLittlefsFs() { return LittlefsFs(); }

LittlefsFs LITTLEFS = CreateLittlefsFs();

}  // namespace roo_io

#endif
