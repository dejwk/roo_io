#pragma once

#ifdef ARDUINO

#include <Arduino.h>
#include <SD.h>

#if (defined ESP32 || defined ROO_TESTING)
#include <SPI.h>

#include "roo_io/fs/esp32/base_vfs_filesystem.h"
#else
#include "roo_io/fs/filesystem.h"
#endif

namespace roo_io {

#if (defined ESP32 || defined ROO_TESTING)

class ArduinoSdFs : public BaseEsp32VfsFilesystem {
 public:
  void setCsPin(uint8_t cs_pin) { cs_pin_ = (gpio_num_t)cs_pin; }
  void setSPI(decltype(::SPI)& spi) { spi_ = &spi; }

  MediaPresence checkMediaPresence() override;

 protected:
  friend ArduinoSdFs CreateArduinoSdFs();

  ArduinoSdFs(uint8_t cs_pin = SS, decltype(::SD)& sd = ::SD,
              decltype(::SPI)& spi = ::SPI, uint32_t freq = 20000000);

  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

  gpio_num_t cs_pin_;

  decltype(::SD)& sd_;
  decltype(::SPI)* spi_;
};

#else

class ArduinoSdFs : public Filesystem {
 public:
  MediaPresence checkMediaPresence() override;

  void setCsPin(uint8_t cs_pin) { cs_pin_ = cs_pin; }

  bool readOnly() const { return read_only_; }
  void setReadOnly(bool read_only) { read_only_ = read_only; }

 protected:
  friend ArduinoSdFs CreateArduinoSdFs();

  ArduinoSdFs(uint8_t cs_pin = SS);

  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

  uint8_t cs_pin_;
  decltype(::SD)& sd_;

  bool read_only_;
};

#endif  // ESP32

extern ArduinoSdFs SD;

}  // namespace roo_io

#endif  // ARDUINO
