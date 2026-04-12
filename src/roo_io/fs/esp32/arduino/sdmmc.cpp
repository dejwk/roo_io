#include "roo_io/fs/esp32/arduino/sdmmc.h"

#if (defined ESP32 && defined ARDUINO)

#include "soc/soc_caps.h"

#if SOC_SDMMC_HOST_SUPPORTED

#if !defined(MLOG_roo_io_fs)
#define MLOG_roo_io_fs 0
#endif

#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"

#include <sys/stat.h>

namespace roo_io {

ArduinoSdMmcFs::ArduinoSdMmcFs()
    : BaseEsp32VfsFilesystem(SDMMC_FREQ_HIGHSPEED, "/sdcard"),
      mode_1bit_(true),
      slot_config_(SDMMC_SLOT_CONFIG_DEFAULT()) {}

void ArduinoSdMmcFs::setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0) {
  ::SD_MMC.setPins(pin_clk, pin_cmd, pin_d0);
  mode_1bit_ = true;
#ifdef SOC_SDMMC_USE_GPIO_MATRIX
  slot_config_.clk = (gpio_num_t)pin_clk;
  slot_config_.cmd = (gpio_num_t)pin_cmd;
  slot_config_.d0 = (gpio_num_t)pin_d0;
#endif
  slot_config_.width = 1;
}

void ArduinoSdMmcFs::setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0,
                             uint8_t pin_d1, uint8_t pin_d2, uint8_t pin_d3) {
  ::SD_MMC.setPins(pin_clk, pin_cmd, pin_d0, pin_d1, pin_d2, pin_d3);
  mode_1bit_ = false;
#ifdef SOC_SDMMC_USE_GPIO_MATRIX
  slot_config_.clk = (gpio_num_t)pin_clk;
  slot_config_.cmd = (gpio_num_t)pin_cmd;
  slot_config_.d0 = (gpio_num_t)pin_d0;
  slot_config_.d1 = (gpio_num_t)pin_d1;
  slot_config_.d2 = (gpio_num_t)pin_d2;
  slot_config_.d3 = (gpio_num_t)pin_d3;
#endif
  slot_config_.width = 4;
}

MountImpl::MountResult ArduinoSdMmcFs::mountImpl(
    std::function<void()> unmount_fn) {
  MLOG(roo_io_fs) << "Mounting SD card";
#if defined(ROO_TESTING)
  mount_base_path_ = FakeEsp32().fs_root();
#else
  mount_base_path_.clear();
#endif
  mount_base_path_.append(mountPoint());
  bool result =
      ::SD_MMC.begin(mount_base_path_.c_str(), mode_1bit_,
                     formatIfMountFailed(), frequency(), maxOpenFiles());
  if (!result) {
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mount_base_path_.c_str(), readOnly(), unmount_fn)));
}

void ArduinoSdMmcFs::unmountImpl() {
  MLOG(roo_io_fs) << "Unmounting SD card";
  ::SD_MMC.end();
  mount_base_path_.clear();
}

Filesystem::MediaPresence ArduinoSdMmcFs::checkMediaPresence() {
  if (!mount_base_path_.empty()) {
    // Mounted. Stat the mount path — goes through VFS/FatFs/SDMMC driver,
    // which will fail if the card was physically removed.
    struct stat st;
    return (stat(mount_base_path_.c_str(), &st) == 0) ? kMediaPresent
                                                      : kMediaAbsent;
  }
  // Not mounted. Init the SDMMC host and do the card handshake to probe
  // for card presence, without mounting a filesystem.
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.flags = mode_1bit_ ? SDMMC_HOST_FLAG_1BIT : SDMMC_HOST_FLAG_4BIT;
  host.max_freq_khz = frequency() / 1000;

  esp_err_t ret = sdmmc_host_init();
  if (ret != ESP_OK) {
    return kMediaAbsent;
  }

  ret = sdmmc_host_init_slot(host.slot, &slot_config_);
  if (ret != ESP_OK) {
    sdmmc_host_deinit();
    return kMediaAbsent;
  }

  sdmmc_card_t card;
  ret = sdmmc_card_init(&host, &card);

  sdmmc_host_deinit();
  return (ret == ESP_OK) ? kMediaPresent : kMediaAbsent;
}

ArduinoSdMmcFs CreateArduinoSdMmcFs() { return ArduinoSdMmcFs(); }

ArduinoSdMmcFs SD_MMC = CreateArduinoSdMmcFs();

}  // namespace roo_io

#endif
#endif
