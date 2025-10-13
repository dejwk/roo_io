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

  void setCsPin(uint8_t pin_cs);
  void setSpiHost(spi_host_device_t spi_host);

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
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

 private:
  spi_host_device_t spi_host_;
  gpio_num_t pin_cs_;
  uint32_t spi_frequency_;

  std::string mount_point_;
  uint8_t max_open_files_;
  bool format_if_mount_failed_;
  bool read_only_;

  std::string mount_base_path_;
  sdmmc_card_t* card_;
};

extern SdSpiFs SdSpi;

}  // namespace roo_io

#endif  // (defined(ESP32) || defined(ROO_TESTING))