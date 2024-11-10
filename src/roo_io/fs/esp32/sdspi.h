#pragma once

#include "driver/sdmmc_types.h"
#include "hal/spi_types.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {
namespace esp32 {

class SdSpiFs : public Filesystem {
 public:
  SdSpiFs();
  SdSpiFs(uint8_t pin_sck, uint8_t pin_miso, uint8_t pin_mosi, uint8_t pin_cs,
          spi_host_device_t spi_host = HSPI_HOST);

  const char* mount_point() const;
  void set_mount_point(const char* mount_point);

  uint8_t max_files() const;
  void set_max_files(uint8_t max_files);

  bool format_if_empty() const;
  void set_format_if_empty(bool format_if_empty);

  void spi_config(uint8_t pin_sck, uint8_t pin_miso, uint8_t pin_mosi,
                  uint8_t pin_cs, spi_host_device_t spi_host = HSPI_HOST);

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

 private:
  spi_host_device_t spi_host_;
  gpio_num_t pin_sck_;
  gpio_num_t pin_miso_;
  gpio_num_t pin_mosi_;
  gpio_num_t pin_cs_;

  std::string mount_point_;
  uint8_t max_files_;
  bool format_if_empty_;

  uint32_t spi_frequency_;

  std::string mount_base_path_;
  sdmmc_card_t* card_;
};

extern SdSpiFs SDSPI;

}  // namespace esp32
}  // namespace roo_io