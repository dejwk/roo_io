#pragma once

#if (defined ESP_PLATFORM)

#include "soc/soc_caps.h"

#if SOC_SDMMC_HOST_SUPPORTED

#include "driver/sdmmc_types.h"
#include "hal/gpio_types.h"
#include "roo_io/fs/esp32/base_vfs_filesystem.h"

namespace roo_io {

class SdMmcFs : public BaseEsp32VfsFilesystem {
 public:
  void setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0);

  void setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0, uint8_t pin_d1,
               uint8_t pin_d2, uint8_t pin_d3);

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

  std::string mount_base_path_;
  sdmmc_card_t* card_;
  uint8_t pdrv_;
};

extern SdMmcFs SDMMC;

}  // namespace roo_io

#endif  // CONFIG_IDF_TARGET_ESP32S3
#endif  // defined(ESP_PLATFORM)
