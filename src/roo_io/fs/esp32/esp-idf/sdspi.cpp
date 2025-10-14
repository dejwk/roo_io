#include "roo_io/fs/esp32/esp-idf/sdspi.h"

#if defined(ROO_TESTING)

#include "roo_testing/devices/microcontroller/esp32/fake_esp32.h"

#endif

#if (defined(ESP32) || defined(ROO_TESTING))

#if !defined(MLOG_roo_io_fs)
#define MLOG_roo_io_fs 0
#endif

#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_vfs_fat.h"
#include "roo_io/fs/posix/posix_mount.h"
#include "roo_logging.h"

namespace roo_io {

SdSpiFs::SdSpiFs(uint8_t cs_pin, spi_host_device_t spi_host, uint32_t frequency)
    : cs_pin_((gpio_num_t)cs_pin),
      spi_host_(spi_host),
      frequency_(frequency),
      mount_point_("/sd"),
      max_open_files_(5),
      format_if_mount_failed_(false),
      read_only_(false) {}

const char* SdSpiFs::mountPoint() const { return mount_point_.c_str(); }

void SdSpiFs::setMountPoint(const char* mount_point) {
  mount_point_ = mount_point;
}

uint8_t SdSpiFs::maxOpenFiles() const { return max_open_files_; }

void SdSpiFs::setMaxOpenFiles(uint8_t max_open_files) {
  max_open_files_ = max_open_files;
}

bool SdSpiFs::formatIfMountFailed() const { return format_if_mount_failed_; }

void SdSpiFs::setFormatIfMountFailed(bool format_if_mount_failed) {
  format_if_mount_failed_ = format_if_mount_failed;
}

bool SdSpiFs::readOnly() const { return read_only_; }

void SdSpiFs::setReadOnly(bool read_only) { read_only_ = read_only; }

MountImpl::MountResult SdSpiFs::mountImpl(std::function<void()> unmount_fn) {
  MLOG(roo_io_fs) << "Mounting SD card";
#if defined(ROO_TESTING)
  mount_base_path_ = FakeEsp32().fs_root();
#else
  mount_base_path_.clear();
#endif
  mount_base_path_.append(mount_point_);

  esp_err_t ret;

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();

  sdspi_device_config_t dev_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  dev_config.host_id = spi_host_;
  dev_config.gpio_cs = cs_pin_;

  esp_vfs_fat_mount_config_t mount_config = {
      .format_if_mount_failed = format_if_mount_failed_,
      .max_files = max_open_files_,
      .allocation_unit_size = 16 * 1024};

  ret = esp_vfs_fat_sdspi_mount(mount_base_path_.c_str(), &host, &dev_config,
                                &mount_config, &card_);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      LOG(ERROR) << "Failed to mount filesystem. If you want the card to be "
                    "formatted, use setFormatIfEmpty(true)";
      return MountImpl::MountError(kGenericMountError);
    } else if (ret == ESP_ERR_TIMEOUT) {
      return MountImpl::MountError(kNoMedia);
    } else {
      LOG(ERROR) << "Failed to initialize the card (" << esp_err_to_name(ret)
                 << ").";
    }
    return MountImpl::MountError(kGenericMountError);
  }

  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mount_base_path_.c_str(), read_only_, unmount_fn)));
}

void SdSpiFs::unmountImpl() {
  MLOG(roo_io_fs) << "Unmounting SD card";
  CHECK_NOTNULL(card_);

  esp_err_t ret = esp_vfs_fat_sdcard_unmount(mount_base_path_.c_str(), card_);
  if (ret != ESP_OK) {
    LOG(ERROR) << "Unmounting card failed: " << esp_err_to_name(ret);
  }
  card_ = nullptr;
  mount_base_path_.clear();
}

Filesystem::MediaPresence SdSpiFs::checkMediaPresence() {
  return kMediaPresenceUnknown;
}

SdSpiFs CreateSdSpiFs() { return SdSpiFs(); }

SdSpiFs SDSPI = CreateSdSpiFs();

}  // namespace roo_io

#endif
