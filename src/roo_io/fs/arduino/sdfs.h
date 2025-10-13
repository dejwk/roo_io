#pragma once

#ifdef ARDUINO

#include <SD.h>
#include <SPI.h>

#include <memory>

#include "roo_io/fs/filesystem.h"

namespace roo_io {

#ifdef ESP32

class ArduinoSdFs : public Filesystem {
 public:
  ArduinoSdFs(uint8_t cs_pin = SS, decltype(::SD)& sd = ::SD,
              decltype(::SPI)& spi = ::SPI, uint32_t freq = 20000000);

  MediaPresence checkMediaPresence() override;

  void setCsPin(uint8_t cs_pin) { cs_pin_ = cs_pin; }

  void setSPI(decltype(::SPI)& spi) { spi_ = &spi; }
  void setFrequency(uint32_t freq) { frequency_ = freq; }

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

  uint8_t cs_pin_;

  decltype(::SD)& sd_;
  decltype(::SPI)* spi_;
  uint32_t frequency_;
};

#else

class ArduinoSdFs : public Filesystem {
 public:
  ArduinoSdFs(uint8_t cs_pin = SS);

  MediaPresence checkMediaPresence() override;

  void setCsPin(uint8_t cs_pin) { cs_pin_ = cs_pin; }

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

  uint8_t cs_pin_;
  decltype(::SD)& sd_;
};

#endif

extern ArduinoSdFs SD;

}  // namespace roo_io

#endif  // ARDUINO
