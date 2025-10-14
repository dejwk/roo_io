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
    : BaseEsp32VfsFilesystem(freq, "/sd"),
      cs_pin_((gpio_num_t)cs_pin),
      spi_(&spi),
      pdrv_(0xFF) {}

MountImpl::MountResult ArduinoSdSpiFs::mountImpl(
    std::function<void()> unmount_fn) {
  pdrv_ = sdcard_init(cs_pin_, spi_, frequency());
  if (pdrv_ == 0xFF) {
    return MountImpl::MountError(kGenericMountError);
  }
  if (!sdcard_mount(pdrv_, mountPoint(), maxOpenFiles(),
                    formatIfMountFailed())) {
    sdcard_unmount(pdrv_);
    sdcard_uninit(pdrv_);
    pdrv_ = 0xFF;
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mountPoint(), readOnly(), unmount_fn)));
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
