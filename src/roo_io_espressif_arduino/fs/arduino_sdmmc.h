#pragma once

#ifdef ESP32

#include <Arduino.h>
#include <SD_MMC.h>

#ifdef SOC_SDMMC_HOST_SUPPORTED

#include <functional>
#include <string>

#include "roo_io/base/string_view.h"
#include "roo_io/fs/filesystem.h"
#include "roo_io/fs/posix/posix_mount.h"

namespace roo_io {
namespace esp32 {

// Exposes the roo_io::Filesystem interface over the native ESP32 Arduino SD
// implementation, using the VFS API and Posix interface to the file system.
class SdMmcFs : public Filesystem {
 public:
  SdMmcFs(uint8_t clk, uint8_t cmd, uint8_t d0,
          const char* mountpoint = "/sdcard",
          int frequency = SDMMC_FREQ_HIGHSPEED, uint8_t max_open_files = 5);

  SdMmcFs(uint8_t clk, uint8_t cmd, uint8_t d0, uint8_t d1, uint8_t d2,
          uint8_t d3, const char* mountpoint = "/sdcard",
          int frequency = SDMMC_FREQ_HIGHSPEED, uint8_t max_open_files = 5);

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

extern SdMmcFs SdMmc;

}  // namespace esp32
}  // namespace roo_io

#endif
#endif