#pragma once

#if (defined ESP32 && defined ARDUINO)

#include "soc/soc_caps.h"

#if SOC_SDMMC_HOST_SUPPORTED

#include <Arduino.h>
#include <SD_MMC.h>

#include <functional>
#include <string>

#include "roo_io/base/string_view.h"
#include "roo_io/fs/filesystem.h"
#include "roo_io/fs/posix/posix_mount.h"

namespace roo_io {

// Exposes the roo_io::Filesystem interface over the native ESP32 Arduino SD
// implementation, using the VFS API and Posix interface to the file system.
// Internally uses the SD_MMC object. If you don't care to use the SD_MMC object
// directly, you might as well use the esp-idf/sdmmc.h version.
class ArduinoSdMmcFs : public Filesystem {
 public:
  ArduinoSdMmcFs(uint8_t clk, uint8_t cmd, uint8_t d0,
                 const char* mountpoint = "/sdcard",
                 int frequency = SDMMC_FREQ_HIGHSPEED,
                 uint8_t max_open_files = 5);

  ArduinoSdMmcFs(uint8_t clk, uint8_t cmd, uint8_t d0, uint8_t d1, uint8_t d2,
                 uint8_t d3, const char* mountpoint = "/sdcard",
                 int frequency = SDMMC_FREQ_HIGHSPEED,
                 uint8_t max_open_files = 5);

  MediaPresence checkMediaPresence() override;

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

 private:
  uint8_t clk_;
  uint8_t cmd_;
  uint8_t d0_;
  uint8_t d1_;
  uint8_t d2_;
  uint8_t d3_;
  bool mode_1bit_;

  std::string mountpoint_;
  uint32_t frequency_;
  uint8_t max_open_files_;
  bool read_only_;
};

// extern ArduinoSdMmcFs SdMmc;

}  // namespace roo_io

#endif  // SOC_SDMMC_HOST_SUPPORTED
#endif  // (defined ESP32 && defined ARDUINO)