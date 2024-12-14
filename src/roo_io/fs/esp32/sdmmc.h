#pragma once

#include "driver/sdmmc_types.h"
#include "hal/gpio_types.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {
namespace esp32 {

class SdMmcFs : public Filesystem {
 public:
  void setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0);

  void setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0, uint8_t pin_d1,
               uint8_t pin_d2, uint8_t pin_d3);

  const char* mountPoint() const;
  void setMountPoint(const char* mount_point);

  uint8_t maxOpenFiles() const;
  void setMaxOpenFiles(uint8_t max_files);

  bool formatIfEmpty() const;
  void setFormatIfEmpty(bool format_if_empty);

  //   void spi_config(uint8_t pin_sck, uint8_t pin_miso, uint8_t pin_mosi,
  //                   uint8_t pin_cs, spi_host_device_t spi_host);

  MediaPresence checkMediaPresence() override;

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

 private:
  friend SdMmcFs CreateSdMmcFs();

  SdMmcFs();

  bool use_default_pins_;
  gpio_num_t pin_clk_;
  gpio_num_t pin_cmd_;
  gpio_num_t pin_d0_;
  gpio_num_t pin_d1_;
  gpio_num_t pin_d2_;
  gpio_num_t pin_d3_;
  uint8_t width_;

  std::string mount_point_;
  uint8_t max_open_files_;
  bool format_if_empty_;

  int frequency_;

  std::string mount_base_path_;
  sdmmc_card_t* card_;
};

extern SdMmcFs SDMMC;

}  // namespace esp32
}  // namespace roo_io