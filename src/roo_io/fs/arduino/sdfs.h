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
  MediaPresence checkMediaPresence() override;

  void setCsPin(uint8_t cs_pin) { cs_pin_ = cs_pin; }

  void setSPI(decltype(::SPI)& spi) { spi_ = &spi; }
  void setFrequency(uint32_t freq) { frequency_ = freq; }

  const char* mountPoint() const;
  void setMountPoint(const char* mount_point);

  uint8_t maxOpenFiles() const;
  void setMaxOpenFiles(uint8_t max_open_files);

  bool formatIfMountFailed() const;
  void setFormatIfMountFailed(bool format_if_mount_failed);

  bool readOnly() const { return read_only_; }
  void setReadOnly(bool read_only) { read_only_ = read_only; }

 protected:
  friend ArduinoSdFs CreateArduinoSdFs();

  ArduinoSdFs(uint8_t cs_pin = SS, decltype(::SD)& sd = ::SD,
              decltype(::SPI)& spi = ::SPI, uint32_t freq = 20000000);

  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

  uint8_t cs_pin_;

  decltype(::SD)& sd_;
  decltype(::SPI)* spi_;
  uint32_t frequency_;

  std::string mount_point_;
  uint8_t max_open_files_;
  bool format_if_mount_failed_;
  bool read_only_;
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

#endif

extern ArduinoSdFs SD;

}  // namespace roo_io

#endif  // ARDUINO
