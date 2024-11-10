#include "roo_io/fs/arduino/arduino_sdfs.h"

namespace roo_io {

ArduinoSdFs::ArduinoSdFs(uint8_t ss_pin, SDFS& sd, SPIClass& spi, uint32_t freq)
    : sd_(sd), spi_(spi), ss_pin_(ss_pin), frequency_(freq) {
  pinMode(ss_pin, OUTPUT);
  digitalWrite(ss_pin, HIGH);
}

MountImpl::MountResult ArduinoSdFs::mountImpl(
    std::function<void()> unmount_fn) {
  digitalWrite(ss_pin_, LOW);
  if (!sd_.begin(ss_pin_, spi_, frequency_)) {
    digitalWrite(ss_pin_, HIGH);
    return MountImpl::MountError(kMountError);
  }
  return MountImpl::Mounted(
      std::unique_ptr<MountImpl>(new ArduinoMountImpl(sd_, false, unmount_fn)));
}

void ArduinoSdFs::unmountImpl() {
  sd_.end();
  digitalWrite(ss_pin_, HIGH);
}

ArduinoSdFs SD;

}  // namespace roo_io