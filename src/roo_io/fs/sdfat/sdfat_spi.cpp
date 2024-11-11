#include "roo_io/fs/sdfat/sdfat_spi.h"

namespace roo_io {

SdFatSpiFs::SdFatSpiFs(uint8_t sck_pin, uint8_t miso_pin, uint8_t mosi_pin,
                       uint8_t ss_pin, SdFat& sd, decltype(SPI)& spi,
                       uint32_t freq)
    : sd_(sd),
      spi_(spi),
      sck_pin_(sck_pin),
      miso_pin_(miso_pin),
      mosi_pin_(mosi_pin),
      ss_pin_(ss_pin),
      frequency_(freq) {}

MountImpl::MountResult SdFatSpiFs::mountImpl(std::function<void()> unmount_fn) {
  long time = millis();
  if (!sd_.begin(SdSpiConfig(ss_pin_, SHARED_SPI, frequency_, &spi_))) {
    return MountImpl::MountError(kMountError);
  }
  return MountImpl::Mounted(
      std::unique_ptr<MountImpl>(new SdFatMountImpl(sd_, false, unmount_fn)));
}

void SdFatSpiFs::unmountImpl() {
  sd_.end();
  // SdFat calls spi.close() on sd_.end(), deiniting the entire bus. To work
  // around that, we reinit the bus right away.
  spi_.begin(sck_pin_, miso_pin_, mosi_pin_);
}

// SdFatFs SDFAT;

}  // namespace roo_io