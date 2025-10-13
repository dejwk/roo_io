#include "roo_io/fs/esp32/arduino/sdmmc.h"

#if (defined ESP32 && defined ARDUINO)

#include "soc/soc_caps.h"

#if SOC_SDMMC_HOST_SUPPORTED

#if !defined(MLOG_roo_io_fs)
#define MLOG_roo_io_fs 0
#endif

namespace roo_io {

ArduinoSdMmcFs::ArduinoSdMmcFs()
    : mode_1bit_(true),
      mount_point_("/sdcard"),
      max_open_files_(5),
      format_if_mount_failed_(false),
      frequency_(SDMMC_FREQ_HIGHSPEED) {}

void ArduinoSdMmcFs::setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0) {
  SD_MMC.setPins(pin_clk, pin_cmd, pin_d0);
  mode_1bit_ = true;
}

void ArduinoSdMmcFs::setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0,
                             uint8_t pin_d1, uint8_t pin_d2, uint8_t pin_d3) {
  SD_MMC.setPins(pin_clk, pin_cmd, pin_d0, pin_d1, pin_d2, pin_d3);
  mode_1bit_ = false;
}

const char* ArduinoSdMmcFs::mountPoint() const { return mount_point_.c_str(); }

void ArduinoSdMmcFs::setMountPoint(const char* mountpoint) {
  mount_point_ = mountpoint;
}

uint8_t ArduinoSdMmcFs::maxOpenFiles() const { return max_open_files_; }

void ArduinoSdMmcFs::setMaxOpenFiles(uint8_t max_open_files) {
  max_open_files_ = max_open_files;
}

bool ArduinoSdMmcFs::formatIfMountFailed() const {
  return format_if_mount_failed_;
}

void ArduinoSdMmcFs::setFormatIfMountFailed(bool format_if_mount_failed) {
  format_if_mount_failed_ = format_if_mount_failed;
}

bool ArduinoSdMmcFs::readOnly() const { return read_only_; }

void ArduinoSdMmcFs::setReadOnly(bool read_only) { read_only_ = read_only; }

MountImpl::MountResult ArduinoSdMmcFs::mountImpl(
    std::function<void()> unmount_fn) {
  MLOG(roo_io_fs) << "Mounting SD card";
#if defined(ROO_TESTING)
  mount_base_path_ = FakeEsp32().fs_root();
#else
  mount_base_path_.clear();
#endif
  mount_base_path_.append(mount_point_);
  bool result =
      ::SD_MMC.begin(mount_base_path_.c_str(), mode_1bit_,
                     format_if_mount_failed_, frequency_, max_open_files_);
  if (!result) {
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mount_base_path_.c_str(), read_only_, unmount_fn)));
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
