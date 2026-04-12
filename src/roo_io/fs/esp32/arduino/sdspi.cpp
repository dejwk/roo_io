#include "roo_io/fs/esp32/arduino/sdspi.h"

#if (defined ESP32 && defined ARDUINO)

#include "FS.h"
#include "SD.h"
#include "diskio_impl.h"
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
  if (pdrv_ != 0xFF) {
    // Drive is registered. Send CMD13 to check if card is still responsive.
    // disk_status (ff_sd_status) is non-destructive and won't disrupt a mount.
    return (disk_status(pdrv_) & STA_NOINIT) ? kMediaAbsent : kMediaPresent;
  }
  // Not initialized; register a drive and run the SPI handshake to probe for
  // card presence, without mounting a filesystem.
  uint8_t pdrv = sdcard_init(cs_pin_, spi_, frequency());
  if (pdrv == 0xFF) {
    return kMediaAbsent;
  }
  // sdcard_init only registers the drive; disk_initialize actually talks to
  // the card over SPI and sets the card type.
  disk_initialize(pdrv);
  bool present = sdcard_type(pdrv) != CARD_NONE;
  sdcard_uninit(pdrv);
  return present ? kMediaPresent : kMediaAbsent;
}

ArduinoSdSpiFs CreateArduinoSdSpiFs() { return ArduinoSdSpiFs(); }

ArduinoSdSpiFs SD_SPI = CreateArduinoSdSpiFs();

}  // namespace roo_io

#endif
