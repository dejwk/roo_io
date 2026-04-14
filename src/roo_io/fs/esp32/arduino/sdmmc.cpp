#include "roo_io/fs/esp32/arduino/sdmmc.h"

#if (defined ESP32 && defined ARDUINO)

#include "soc/soc_caps.h"

#if SOC_SDMMC_HOST_SUPPORTED

#if !defined(MLOG_roo_io_fs)
#define MLOG_roo_io_fs 0
#endif

#include "diskio_impl.h"
#include "diskio_sdmmc.h"
#include "driver/sdmmc_host.h"
#include "roo_io/fs/esp32/internal/sd_mmc_probe.h"
#include "sdmmc_cmd.h"

namespace roo_io {

ArduinoSdMmcFs::ArduinoSdMmcFs()
    : BaseEsp32VfsFilesystem((uint32_t)SDMMC_FREQ_HIGHSPEED * 1000L, "/sdcard"),
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
  if (checkMediaPresence() == kMediaAbsent) {
    return MountImpl::MountError(kNoMedia);
  }
  mount_base_path_.clear();
#endif
  mount_base_path_.append(mountPoint());
  // Note: SD_MMC expects frequency in kHz, not Hz.
  bool result =
      ::SD_MMC.begin(mount_base_path_.c_str(), mode_1bit_,
                     formatIfMountFailed(), frequency() / 1000, maxOpenFiles());
  if (!result) {
    mount_base_path_.clear();
    return MountImpl::MountError(kGenericMountError);
  }
  // Enable disk status checking so disk_status(0) sends CMD13.
  ff_sdmmc_set_disk_status_check(0, true);
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mount_base_path_.c_str(), readOnly(), unmount_fn)));
}

void ArduinoSdMmcFs::unmountImpl() {
  MLOG(roo_io_fs) << "Unmounting SD card";
  ::SD_MMC.end();
  mount_base_path_.clear();
}

Filesystem::MediaPresence ArduinoSdMmcFs::checkMediaPresence() {
#if defined(ROO_TESTING)
  return kMediaPresenceUnknown;
#else
  if (!mount_base_path_.empty()) {
    // Already mounted.  disk_status(0) calls sdmmc_get_status() internally
    // (CMD13) when status checking is enabled — fast, non-destructive.
    return (disk_status(0) == 0) ? kMediaPresent : kMediaAbsent;
  }
  // Not mounted.  Full init → CMD8 → deinit cycle (~2–3 ms).
  return internal::SdMmcProbe(SDMMC_HOST_SLOT_1, &slot_config_) ? kMediaPresent
                                                                : kMediaAbsent;
#endif
}

ArduinoSdMmcFs CreateArduinoSdMmcFs() { return ArduinoSdMmcFs(); }

ArduinoSdMmcFs SD_MMC = CreateArduinoSdMmcFs();

}  // namespace roo_io

#endif
#endif
