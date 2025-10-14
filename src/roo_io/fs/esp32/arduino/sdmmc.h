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
  void setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0);

  void setPins(uint8_t pin_clk, uint8_t pin_cmd, uint8_t pin_d0, uint8_t pin_d1,
               uint8_t pin_d2, uint8_t pin_d3);

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
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

 private:
  friend ArduinoSdMmcFs CreateArduinoSdMmcFs();

  ArduinoSdMmcFs();

  bool mode_1bit_;
  uint32_t frequency_;

  std::string mount_point_;
  uint8_t max_open_files_;
  bool format_if_mount_failed_;
  bool read_only_;

  std::string mount_base_path_;
};

extern ArduinoSdMmcFs SD_MMC;

}  // namespace roo_io

#endif  // SOC_SDMMC_HOST_SUPPORTED
#endif  // (defined ESP32 && defined ARDUINO)