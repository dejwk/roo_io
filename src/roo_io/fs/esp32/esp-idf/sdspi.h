#pragma once

#if (defined(ESP_PLATFORM) || defined(ROO_TESTING))

#include "driver/sdmmc_types.h"
#include "hal/gpio_types.h"
#include "hal/spi_types.h"
#include "roo_io/fs/esp32/base_vfs_filesystem.h"

namespace roo_io {

// For use with esp-idf.
//
// You can use it in Arduino, as long as you don't have other Arduino devices on
// the same SPI bus. Otherwise, use "roo_io/fs/esp32/arduino/sdfs.h" instead.
class SdSpiFs : public BaseEsp32VfsFilesystem {
 public:
  void setCsPin(uint8_t cs_pin) { cs_pin_ = (gpio_num_t)cs_pin; }
  void setSpiHost(spi_host_device_t spi_host) { spi_host_ = spi_host; }

  MediaPresence checkMediaPresence() override;

 protected:
  friend SdSpiFs CreateSdSpiFs();

  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  SdSpiFs(uint8_t cs_pin = -1, spi_host_device_t spi_host = SPI2_HOST,
          uint32_t freq = 20000000);

  void unmountImpl() override;

 private:
  gpio_num_t cs_pin_;

  spi_host_device_t spi_host_;

  std::string mount_base_path_;
  sdmmc_card_t* card_;
};

extern SdSpiFs SDSPI;

}  // namespace roo_io

#endif  // (defined(ESP_PLATFORM) || defined(ROO_TESTING))