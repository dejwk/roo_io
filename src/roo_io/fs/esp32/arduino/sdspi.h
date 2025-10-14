#pragma once

#if (defined ESP32 && defined ARDUINO)

#include <functional>
#include <string>

#include "SPI.h"
#include "roo_io/base/string_view.h"
#include "roo_io/fs/esp32/base_vfs_filesystem.h"
#include "roo_io/fs/posix/posix_mount.h"
#include "sd_diskio.h"

namespace roo_io {

// Exposes the roo_io::Filesystem interface over the native ESP32 Arduino SD
// implementation, using the VFS API and Posix interface to the file system.
// This is very similar to "roo_io/fs/arduino/sdfs.h", but uses the lower-level
// sd_diskio interface directly, instead of going through the Arduino SD object.
class ArduinoSdSpiFs : public BaseEsp32VfsFilesystem {
 public:
  void setCsPin(uint8_t cs_pin) { cs_pin_ = (gpio_num_t)cs_pin; }
  void setSPI(decltype(::SPI)& spi) { spi_ = &spi; }

  MediaPresence checkMediaPresence() override;

 protected:
  friend ArduinoSdSpiFs CreateArduinoSdSpiFs();

  ArduinoSdSpiFs(uint8_t cs_pin = SS, decltype(::SPI)& spi = ::SPI,
                 uint32_t freq = 20000000);

  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

  gpio_num_t cs_pin_;

  decltype(::SPI)* spi_;

  uint8_t pdrv_;
};

extern ArduinoSdSpiFs SD_SPI;

}  // namespace roo_io

#endif
