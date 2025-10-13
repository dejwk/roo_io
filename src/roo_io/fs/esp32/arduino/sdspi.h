#pragma once

#if (defined ESP32 && defined ARDUINO)

#include <functional>
#include <string>

#include "SPI.h"
#include "roo_io/base/string_view.h"
#include "roo_io/fs/filesystem.h"
#include "roo_io/fs/posix/posix_mount.h"
#include "sd_diskio.h"

namespace roo_io {

// Exposes the roo_io::Filesystem interface over the native ESP32 Arduino SD
// implementation, using the VFS API and Posix interface to the file system.
class ArduinoSdSpiFs : public Filesystem {
 public:
  void setCsPin(uint8_t pin_cs);
  void setSpi(SPIClass& spi);

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
  friend ArduinoSdSpiFs CreateArduinoSdSpiFs();

  ArduinoSdSpiFs();
//   ArduinoSdSpiFs(uint8_t cs, SPIClass& spi = SPI, uint32_t frequency = 4000000,
//                  roo::string_view mountpoint = "/sd",
//                  uint8_t max_open_files = 5, bool read_only = false);


  SPIClass& spi_;
  uint8_t cs_;
  uint32_t frequency_;

  std::string mount_point_;
  uint8_t max_open_files_;
  bool format_if_mount_failed_;
  bool read_only_;

  uint8_t pdrv_;
};

extern ArduinoSdSpiFs SD_SPI;

}  // namespace roo_io

#endif
