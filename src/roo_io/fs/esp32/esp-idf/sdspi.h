#pragma once

#if (defined(ESP32) || defined(ROO_TESTING))

#include "driver/sdmmc_types.h"
#include "hal/gpio_types.h"
#include "hal/spi_types.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

// For use with esp-idf.
//
// You can use it in Arduino, as long as you don't have other Arduino devices on
// the same SPI bus. Otherwise, use "roo_io/fs/esp32/arduino/sdfs.h" instead.
class SdSpiFs : public Filesystem {
 public:
  SdSpiFs(uint8_t pin_cs = -1, spi_host_device_t spi_host = SPI2_HOST);

  const char* mountPoint() const;
  void setMountPoint(const char* mount_point);

  uint8_t maxFiles() const;
  void setMaxFiles(uint8_t max_files);

  bool formatIfEmpty() const;
  void setFormatIfEmpty(bool format_if_empty);

  void setCsPin(uint8_t pin_cs);
  void setSpiHost(spi_host_device_t spi_host);

  MediaPresence checkMediaPresence() override;

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

 private:
  spi_host_device_t spi_host_;
  //   gpio_num_t pin_sck_;
  //   gpio_num_t pin_miso_;
  //   gpio_num_t pin_mosi_;
  gpio_num_t pin_cs_;

  std::string mount_point_;
  uint8_t max_files_;
  bool format_if_empty_;

  uint32_t spi_frequency_;

  std::string mount_base_path_;
  sdmmc_card_t* card_;
};

extern SdSpiFs SDSPI;

}  // namespace roo_io

#endif  // (defined(ESP32) || defined(ROO_TESTING))