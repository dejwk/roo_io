#include "roo_io/fs/esp32/arduino/sdspi.h"

#if (defined ESP32 && defined ARDUINO)

#include "FS.h"
#include "SD.h"
#include "ff.h"
#include "sd_diskio.h"
#include "vfs_api.h"

namespace roo_io {

ArduinoSdSpiFs::ArduinoSdSpiFs(uint8_t cs_pin, decltype(::SPI)& spi,
                               uint32_t freq)
    : cs_pin_((gpio_num_t)cs_pin),
      spi_(&spi),
      frequency_(freq),
      mount_point_("/sd"),
      max_open_files_(5),
      format_if_mount_failed_(false),
      read_only_(false),
      pdrv_(0xFF) {}

const char* ArduinoSdSpiFs::mountPoint() const { return mount_point_.c_str(); }

void ArduinoSdSpiFs::setMountPoint(const char* mountpoint) {
  mount_point_ = mountpoint;
}

uint8_t ArduinoSdSpiFs::maxOpenFiles() const { return max_open_files_; }

void ArduinoSdSpiFs::setMaxOpenFiles(uint8_t max_open_files) {
  max_open_files_ = max_open_files;
}

bool ArduinoSdSpiFs::formatIfMountFailed() const {
  return format_if_mount_failed_;
}

void ArduinoSdSpiFs::setFormatIfMountFailed(bool format_if_mount_failed) {
  format_if_mount_failed_ = format_if_mount_failed;
}

MountImpl::MountResult ArduinoSdSpiFs::mountImpl(
    std::function<void()> unmount_fn) {
  pdrv_ = sdcard_init(cs_pin_, spi_, frequency_);
  if (pdrv_ == 0xFF) {
    return MountImpl::MountError(kGenericMountError);
  }
  if (!sdcard_mount(pdrv_, mount_point_.c_str(), max_open_files_,
                    format_if_mount_failed_)) {
    sdcard_unmount(pdrv_);
    sdcard_uninit(pdrv_);
    pdrv_ = 0xFF;
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mount_point_.c_str(), read_only_, unmount_fn)));
}

void ArduinoSdSpiFs::unmountImpl() {
  if (pdrv_ != 0xFF) {
    sdcard_unmount(pdrv_);

    sdcard_uninit(pdrv_);
    pdrv_ = 0xFF;
  }
}

Filesystem::MediaPresence ArduinoSdSpiFs::checkMediaPresence() {
  FATFS* fsinfo;
  DWORD fre_clust;
  if (f_getfree("0:", &fre_clust, &fsinfo) != 0) {
    return Filesystem::kMediaAbsent;
  }
  return (fsinfo->csize == 0) ? Filesystem::kMediaAbsent
                              : Filesystem::kMediaPresent;
}

ArduinoSdSpiFs CreateArduinoSdSpiFs() { return ArduinoSdSpiFs(); }

ArduinoSdSpiFs SD_SPI = CreateArduinoSdSpiFs();

}  // namespace roo_io

#endif
