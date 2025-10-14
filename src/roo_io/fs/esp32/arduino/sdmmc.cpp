#include "roo_io/fs/esp32/arduino/sdmmc.h"

#if (defined ESP32 && defined ARDUINO)

#include "soc/soc_caps.h"

#if SOC_SDMMC_HOST_SUPPORTED

#if !defined(MLOG_roo_io_fs)
#define MLOG_roo_io_fs 0
#endif

namespace roo_io {

ArduinoSdMmcFs::ArduinoSdMmcFs()
    : BaseEsp32VfsFilesystem(SDMMC_FREQ_HIGHSPEED, "/sdcard"),
      mode_1bit_(true) {}

void ArduinoSdMmcFs::setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0) {
  SD_MMC.setPins(pin_clk, pin_cmd, pin_d0);
  mode_1bit_ = true;
}

void ArduinoSdMmcFs::setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0,
                             uint8_t pin_d1, uint8_t pin_d2, uint8_t pin_d3) {
  SD_MMC.setPins(pin_clk, pin_cmd, pin_d0, pin_d1, pin_d2, pin_d3);
  mode_1bit_ = false;
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
  return ::SD_MMC.totalBytes() > 0 ? Filesystem::kMediaPresent
                                   : Filesystem::kMediaAbsent;
}

ArduinoSdMmcFs CreateArduinoSdMmcFs() { return ArduinoSdMmcFs(); }

ArduinoSdMmcFs SD_MMC = CreateArduinoSdMmcFs();

}  // namespace roo_io

#endif
#endif
