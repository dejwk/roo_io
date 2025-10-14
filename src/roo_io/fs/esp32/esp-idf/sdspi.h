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
  void setCsPin(uint8_t cs_pin) { cs_pin_ = (gpio_num_t)cs_pin; }
  void setSpiHost(spi_host_device_t spi_host) { spi_host_ = spi_host; }
  void setFrequency(uint32_t freq) { frequency_ = freq; }

  const char* mountPoint() const;
  void setMountPoint(const char* mount_point);

  uint8_t maxOpenFiles() const;
  void setMaxOpenFiles(uint8_t max_open_files);

  bool formatIfMountFailed() const;
  void setFormatIfMountFailed(bool format_if_mount_failed);

  bool readOnly() const;
  void setReadOnly(bool read_only);

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
  uint32_t frequency_;

  std::string mount_point_;
  uint8_t max_open_files_;
  bool format_if_mount_failed_;
  bool read_only_;

  std::string mount_base_path_;
  sdmmc_card_t* card_;
};

extern SdSpiFs SDSPI;

}  // namespace roo_io

#endif  // (defined(ESP32) || defined(ROO_TESTING))