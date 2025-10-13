#include "roo_io/fs/esp32/esp-idf/sdmmc.h"

#if defined(ROO_TESTING)

#include "roo_testing/devices/microcontroller/esp32/fake_esp32.h"

#endif

#if (defined ESP32 && defined ARDUINO)

#include "soc/soc_caps.h"

#if SOC_SDMMC_HOST_SUPPORTED

#if !defined(MLOG_roo_io_fs)
#define MLOG_roo_io_fs 0
#endif

#include "ff.h"

#include "diskio_sdmmc.h"
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"
#include "roo_io/fs/posix/posix_mount.h"
#include "roo_logging.h"

namespace roo_io {

SdMmcFs::SdMmcFs()
    : use_default_pins_(true),
      pin_clk_((gpio_num_t)-1),
      pin_cmd_((gpio_num_t)-1),
      pin_d0_((gpio_num_t)-1),
      pin_d1_((gpio_num_t)-1),
      pin_d2_((gpio_num_t)-1),
      pin_d3_((gpio_num_t)-1),
      width_(0),
      frequency_(20000000),
      mount_point_("/sdmmc"),
      max_open_files_(5),
      format_if_mount_failed_(false),
      read_only_(false),
      card_(nullptr),
      pdrv_(0xFF) {}

void SdMmcFs::setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0) {
  use_default_pins_ = false;
  pin_clk_ = (gpio_num_t)pin_clk;
  pin_cmd_ = (gpio_num_t)pin_cmd;
  pin_d0_ = (gpio_num_t)pin_d0;
  width_ = 1;
}

void SdMmcFs::setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0,
                      uint8_t pin_d1, uint8_t pin_d2, uint8_t pin_d3) {
  use_default_pins_ = false;
  pin_clk_ = (gpio_num_t)pin_clk;
  pin_cmd_ = (gpio_num_t)pin_cmd;
  pin_d0_ = (gpio_num_t)pin_d0;
  pin_d1_ = (gpio_num_t)pin_d1;
  pin_d2_ = (gpio_num_t)pin_d2;
  pin_d3_ = (gpio_num_t)pin_d3;
  width_ = 4;
}

const char* SdMmcFs::mountPoint() const { return mount_point_.c_str(); }

void SdMmcFs::setMountPoint(const char* mount_point) {
  mount_point_ = mount_point;
}

uint8_t SdMmcFs::maxOpenFiles() const { return max_open_files_; }

void SdMmcFs::setMaxOpenFiles(uint8_t max_open_files) {
  max_open_files_ = max_open_files;
}

bool SdMmcFs::formatIfMountFailed() const { return format_if_mount_failed_; }

void SdMmcFs::setFormatIfMountFailed(bool format_if_mount_failed) {
  format_if_mount_failed_ = format_if_mount_failed;
}

bool SdMmcFs::readOnly() const { return read_only_; }

void SdMmcFs::setReadOnly(bool read_only) { read_only_ = read_only; }

MountImpl::MountResult SdMmcFs::mountImpl(std::function<void()> unmount_fn) {
  MLOG(roo_io_fs) << "Mounting SD card";
#if defined(ROO_TESTING)
  mount_base_path_ = FakeEsp32().fs_root();
#else
  mount_base_path_.clear();
#endif
  mount_base_path_.append(mount_point_);

  sdmmc_host_t host = SDMMC_HOST_DEFAULT();

  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  if (!use_default_pins_) {
    slot_config.clk = pin_clk_;
    slot_config.cmd = pin_cmd_;
    slot_config.d0 = pin_d0_;
    slot_config.d1 = pin_d1_;
    slot_config.d2 = pin_d2_;
    slot_config.d3 = pin_d3_;
    slot_config.width = width_;
  }
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = format_if_mount_failed_,
      .max_files = max_open_files_,
      .allocation_unit_size = 16 * 1024};

  esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_base_path_.c_str(), &host,
                                          &slot_config, &mount_config, &card_);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      LOG(ERROR) << "Failed to mount the filesystem.";
      //  " If you want the card to be formatted, use setFormatIfEmpty(true)";
    } else {
      LOG(ERROR) << "Failed to initialize the card: " << esp_err_to_name(ret);
    }
    return MountImpl::MountError(kGenericMountError);
  }
  pdrv_ = ff_diskio_get_pdrv_card(card_);

  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mount_base_path_.c_str(), read_only_, unmount_fn)));
}

void SdMmcFs::unmountImpl() {
  MLOG(roo_io_fs) << "Unmounting SD card";
  CHECK_NOTNULL(card_);

  esp_err_t ret = esp_vfs_fat_sdcard_unmount(mount_base_path_.c_str(), card_);
  if (ret != ESP_OK) {
    LOG(ERROR) << "Unmounting card failed: " << esp_err_to_name(ret);
  }
  card_ = nullptr;
  pdrv_ = 0xFF;
  mount_base_path_.clear();
}

Filesystem::MediaPresence SdMmcFs::checkMediaPresence() {
  FATFS* fsinfo;
  DWORD fre_clust;
  char drv[3] = {(char)(48 + pdrv_), ':', 0};
  if (f_getfree(drv, &fre_clust, &fsinfo) != 0) {
    return Filesystem::kMediaAbsent;
  }
  return (fsinfo->csize == 0) ? Filesystem::kMediaAbsent
                              : Filesystem::kMediaPresent;
}

SdMmcFs CreateSdMmcFs() { return SdMmcFs(); }

SdMmcFs SdMmc = CreateSdMmcFs();

}  // namespace roo_io

#endif
#endif
