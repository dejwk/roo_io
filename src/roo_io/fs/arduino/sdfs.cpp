
#ifdef ARDUINO

#include <Arduino.h>

#include "roo_io/fs/arduino/sdfs.h"

#ifdef ESP32
// Directly use the lower-level POSIX APIs, bypassing Arduino filesystem stuff
// completely.
#include "roo_io/fs/posix/posix_mount.h"
#else
// Fall back to Arduino filesystem APIs.
#include "roo_io/fs/arduino/mount.h"
#endif

namespace roo_io {

#ifdef ESP32

ArduinoSdFs::ArduinoSdFs(uint8_t cs_pin, decltype(::SD)& sd,
                         decltype(::SPI)& spi, uint32_t freq)

    : cs_pin_(cs_pin), sd_(sd), spi_(&spi), frequency_(freq) {}

MountImpl::MountResult ArduinoSdFs::mountImpl(
    std::function<void()> unmount_fn) {
  if (!sd_.begin(cs_pin_, *spi_, frequency_)) {
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(sd_.mountpoint(), false, unmount_fn)));
}

Filesystem::MediaPresence ArduinoSdFs::checkMediaPresence() {
  return sd_.totalBytes() > 0 ? kMediaPresent : kMediaAbsent;
}

#else

ArduinoSdFs::ArduinoSdFs(uint8_t cs_pin) : sd_(::SD), cs_pin_(cs_pin) {}

MountImpl::MountResult ArduinoSdFs::mountImpl(
    std::function<void()> unmount_fn) {
  if (!sd_.begin(cs_pin_)) {
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(
      std::unique_ptr<MountImpl>(new ArduinoMountImpl(sd_, false, unmount_fn)));
}

Filesystem::MediaPresence ArduinoSdFs::checkMediaPresence() {
  return kMediaPresenceUnknown;
}

#endif

void ArduinoSdFs::unmountImpl() { sd_.end(); }

ArduinoSdFs SD;

}  // namespace roo_io

#endif  // ARDUINO
