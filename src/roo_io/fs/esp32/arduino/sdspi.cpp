#include "roo_io/fs/esp32/arduino/sdspi.h"

#if (defined ESP32 && defined ARDUINO)

#include "FS.h"
#include "SD.h"
#include "roo_io/fs/esp32/arduino/internal/sd_spi_probe.h"
#include "sd_diskio.h"
#include "vfs_api.h"

namespace roo_io {

ArduinoSdSpiFs::ArduinoSdSpiFs(uint8_t cs_pin, decltype(::SPI) &spi,
                               uint32_t freq)
    : BaseEsp32VfsFilesystem(freq, "/sd"), cs_pin_((gpio_num_t)cs_pin),
      spi_(&spi), pdrv_(0xFF) {}

MountImpl::MountResult
ArduinoSdSpiFs::mountImpl(std::function<void()> unmount_fn) {
  if (checkMediaPresence() == kMediaAbsent) {
    return MountImpl::MountError(kNoMedia);
  }
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
    // Drive is registered.  Use a fast direct CMD13 probe instead of
    // disk_status(), which blocks ~500 ms when the card is absent (its
    // sdWait spins on floating-low MISO).
    return internal::SdSpiCheckStatus(*spi_, cs_pin_) ? kMediaPresent
                                                      : kMediaAbsent;
  }
  // Not initialized — do a fast direct SPI probe (CMD0 only, no retries).
  // Much faster than disk_initialize() which retries 3× with ~600 ms waits.
  return internal::SdSpiProbeCard(*spi_, cs_pin_) ? kMediaPresent
                                                  : kMediaAbsent;
}

ArduinoSdSpiFs CreateArduinoSdSpiFs() { return ArduinoSdSpiFs(); }

ArduinoSdSpiFs SD_SPI = CreateArduinoSdSpiFs();

}  // namespace roo_io

#endif
