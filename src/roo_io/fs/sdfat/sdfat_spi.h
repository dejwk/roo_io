#pragma once

#include <SPI.h>
#include <SdFat.h>

#include <memory>

#include "roo_io/fs/filesystem.h"

namespace roo_io {

class SdFatSpiFs : public Filesystem {
 public:
  SdFatSpiFs(uint8_t sck_pin, uint8_t miso_pin, uint8_t mosi_pin,
             uint8_t ss_pin, SdFs& sd, decltype(SPI)& spi = SPI,
             uint32_t freq = 20000000);

  MediaPresence checkMediaPresence() override;

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

  SdFs& sd_;
  decltype(SPI)& spi_;
  uint8_t sck_pin_;
  uint8_t miso_pin_;
  uint8_t mosi_pin_;
  uint8_t ss_pin_;
  uint32_t frequency_;
};

// extern SdFatFs SDFAT;

}  // namespace roo_io