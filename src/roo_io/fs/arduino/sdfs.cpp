
#ifdef ARDUINO

#include "roo_io/fs/arduino/sdfs.h"

#include <Arduino.h>

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

    : cs_pin_(cs_pin),
      sd_(sd),
      spi_(&spi),
      frequency_(freq),
      mount_point_("/sd"),
      max_open_files_(5),
      format_if_mount_failed_(false),
      read_only_(false) {}

const char* ArduinoSdFs::mountPoint() const { return mount_point_.c_str(); }

void ArduinoSdFs::setMountPoint(const char* mount_point) {
  mount_point_ = mount_point;
}

uint8_t ArduinoSdFs::maxOpenFiles() const { return max_open_files_; }

void ArduinoSdFs::setMaxOpenFiles(uint8_t max_open_files) {
  max_open_files_ = max_open_files;
}

bool ArduinoSdFs::formatIfMountFailed() const {
  return format_if_mount_failed_;
}

void ArduinoSdFs::setFormatIfMountFailed(bool format_if_mount_failed) {
  format_if_mount_failed_ = format_if_mount_failed;
}

MountImpl::MountResult ArduinoSdFs::mountImpl(
    std::function<void()> unmount_fn) {
  if (!sd_.begin(cs_pin_, *spi_, frequency_, mount_point_.c_str(),
                 max_open_files_, format_if_mount_failed_)) {
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(sd_.mountpoint(), read_only_, unmount_fn)));
}

Filesystem::MediaPresence ArduinoSdFs::checkMediaPresence() {
  return sd_.totalBytes() > 0 ? kMediaPresent : kMediaAbsent;
}

#else

ArduinoSdFs::ArduinoSdFs(uint8_t cs_pin)
    : sd_(::SD), cs_pin_(cs_pin), read_only_(false) {}

MountImpl::MountResult ArduinoSdFs::mountImpl(
    std::function<void()> unmount_fn) {
  if (!sd_.begin(cs_pin_)) {
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new ArduinoMountImpl(sd_, read_only_, unmount_fn)));
}

Filesystem::MediaPresence ArduinoSdFs::checkMediaPresence() {
  return kMediaPresenceUnknown;
}

#endif

void ArduinoSdFs::unmountImpl() { sd_.end(); }

ArduinoSdFs CreateArduinoSdFs() { return ArduinoSdFs(); }

ArduinoSdFs SD = CreateArduinoSdFs();

}  // namespace roo_io

#endif  // ARDUINO
