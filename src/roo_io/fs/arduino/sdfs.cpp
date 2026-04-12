
#ifdef ARDUINO

#include "roo_io/fs/arduino/sdfs.h"

#include <Arduino.h>

#if (defined ESP32 || defined ROO_TESTING)
// Directly use the lower-level POSIX APIs, bypassing Arduino filesystem stuff
// completely.
#include "roo_io/fs/esp32/arduino/internal/sd_spi_probe.h"
#include "roo_io/fs/posix/posix_mount.h"
#else
// Fall back to Arduino filesystem APIs.
#include "roo_io/fs/arduino/mount.h"
#endif

namespace roo_io {

#if (defined ESP32 || defined ROO_TESTING)

ArduinoSdFs::ArduinoSdFs(uint8_t cs_pin, decltype(::SD)& sd,
                         decltype(::SPI)& spi, uint32_t freq)
    : BaseEsp32VfsFilesystem(freq, "/sd"),
      cs_pin_((gpio_num_t)cs_pin),
      sd_(sd),
      spi_(&spi) {}

MountImpl::MountResult ArduinoSdFs::mountImpl(
    std::function<void()> unmount_fn) {
  if (checkMediaPresence() == kMediaAbsent) {
    return MountImpl::MountError(kNoMedia);
  }
  if (!sd_.begin(cs_pin_, *spi_, frequency(), mountPoint(), maxOpenFiles(),
                 formatIfMountFailed())) {
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mountPoint(), readOnly(), unmount_fn)));
}

Filesystem::MediaPresence ArduinoSdFs::checkMediaPresence() {
  if (isMounted()) {
    // Use a fast direct CMD13 probe instead of sd_.totalBytes() which goes
    // through disk_status() → sdWait(500) — blocks ~500 ms on card removal.
    return internal::SdSpiCheckStatus(*spi_, cs_pin_) ? kMediaPresent
                                                      : kMediaAbsent;
  }
  // Not mounted — do a fast direct SPI probe (CMD0 only, no retries).
  // Much faster than disk_initialize() which retries 3× with ~600 ms waits.
  return internal::SdSpiProbeCard(*spi_, cs_pin_) ? kMediaPresent
                                                  : kMediaAbsent;
}

#else

ArduinoSdFs::ArduinoSdFs(uint8_t cs_pin)
    : sd_(::SD),
#if defined(ARDUINO_ARCH_RP2040)
      sdfs_(::SDFS),
#endif
      cs_pin_(cs_pin),
      read_only_(false) {
}

MountImpl::MountResult ArduinoSdFs::mountImpl(
    std::function<void()> unmount_fn) {
  if (!sd_.begin(cs_pin_)) {
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
#if defined(ARDUINO_ARCH_RP2040)
      new ArduinoMountImpl(sdfs_, read_only_, unmount_fn)));
#else
      new ArduinoMountImpl(sd_, read_only_, unmount_fn)));
#endif
}

Filesystem::MediaPresence ArduinoSdFs::checkMediaPresence() {
  return kMediaPresenceUnknown;
}

#endif  // ESP32

void ArduinoSdFs::unmountImpl() { sd_.end(); }

ArduinoSdFs CreateArduinoSdFs() { return ArduinoSdFs(); }

ArduinoSdFs SD = CreateArduinoSdFs();

}  // namespace roo_io

#endif  // ARDUINO
