#include "roo_io/fs/pico/arduino/sdspi.h"

#if defined(ARDUINO) && defined(ARDUINO_ARCH_RP2040)
#include <utility>

#include "roo_io/fs/arduino/internal/sd_spi_probe.h"
#include "roo_io/fs/arduino/mount.h"

namespace roo_io {

PicoSdSpiFs::PicoSdSpiFs(uint8_t cs_pin, decltype(::SPI)& spi,
                         uint32_t frequency)
    : spi_(&spi), frequency_(frequency), cs_pin_(cs_pin) {}

Filesystem::MediaPresence PicoSdSpiFs::checkMediaPresence() {
  // Detection also works before the first mount. begin() uses the caller's
  // configured SPI pins; ownership remains with the application.
  spi_->begin();
  const bool present = isMounted() ? internal::SdSpiCheckStatus(*spi_, cs_pin_)
                                   : internal::SdSpiProbeCard(*spi_, cs_pin_);
  return present ? kMediaPresent : kMediaAbsent;
}

MountImpl::MountResult PicoSdSpiFs::mountImpl(
    std::function<void()> unmount_fn) {
  if (checkMediaPresence() == kMediaAbsent) {
    return MountImpl::MountError(kNoMedia);
  }
  if (frequency_ == 0 || !::SD.begin(cs_pin_, frequency_, *spi_)) {
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new ArduinoMountImpl(::SDFS, read_only_, std::move(unmount_fn))));
}

void PicoSdSpiFs::unmountImpl() {
  // SD and other peripherals may share this application-owned controller.
  ::SD.end(false);
}

PicoSdSpiFs CreatePicoSdSpiFs() { return PicoSdSpiFs(); }

PicoSdSpiFs SD = CreatePicoSdSpiFs();

}  // namespace roo_io
#endif
