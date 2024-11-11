#include "roo_io/fs/arduino/arduino_sdfs.h"

namespace roo_io {

ArduinoSdFs::ArduinoSdFs(uint8_t ss_pin, SDFS& sd, decltype(SPI)& spi,
                         uint32_t freq)
    : sd_(sd), spi_(spi), ss_pin_(ss_pin), frequency_(freq) {}

MountImpl::MountResult ArduinoSdFs::mountImpl(
    std::function<void()> unmount_fn) {
  if (!sd_.begin(ss_pin_, spi_, frequency_)) {
    return MountImpl::MountError(kMountError);
  }
  return MountImpl::Mounted(
      std::unique_ptr<MountImpl>(new ArduinoMountImpl(sd_, false, unmount_fn)));
}

void ArduinoSdFs::unmountImpl() { sd_.end(); }

ArduinoSdFs SD;

}  // namespace roo_io