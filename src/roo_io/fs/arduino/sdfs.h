#pragma once

// Filesystem based on the Arduino SD library. On ESP32, it bypasses most of
// the Arduino layer and uses the underlying VFS filesystem directly.
//
// Supports dynamic mounting, and software read-only mounting.
//
// On ESP32, very similar functionality is provided by the
// roo_io::ArduinoSdSpiFs class, which uses the lower-level sd_diskio interface
// directly, instead of going through the Arduino SD object.
//
// NOTE: the default frequency is set to 4 MHz, which is safe for jumper wires.
// You can increase it (20 MHz is a fair game) if your wiring is solid, using
// setFrequency().

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

/// Arduino SD wrapper that mounts media through the ESP32 VFS layer.
class ArduinoSdFs : public BaseEsp32VfsFilesystem {
 public:
  /// Sets the SD chip-select pin.
  void setCsPin(uint8_t cs_pin) { cs_pin_ = (gpio_num_t)cs_pin; }
  /// Selects the SPI peripheral used for SD access.
  void setSPI(decltype(::SPI)& spi) { spi_ = &spi; }

  /// Probes whether removable media is present.
  MediaPresence checkMediaPresence() override;

 protected:
  friend ArduinoSdFs CreateArduinoSdFs();

  ArduinoSdFs(uint8_t cs_pin = SS, decltype(::SD)& sd = ::SD,
              decltype(::SPI)& spi = ::SPI, uint32_t freq = 4000000);

  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

  gpio_num_t cs_pin_;

  decltype(::SD)& sd_;
  decltype(::SPI)* spi_;
};

#else

/// Arduino SD wrapper for non-ESP32 Arduino platforms.
class ArduinoSdFs : public Filesystem {
 public:
  /// Probes whether removable media is present.
  MediaPresence checkMediaPresence() override;

  /// Sets the SD chip-select pin.
  void setCsPin(uint8_t cs_pin) { cs_pin_ = cs_pin; }

  /// Returns whether newly opened mounts are forced read-only.
  bool readOnly() const { return read_only_; }
  /// Sets whether newly opened mounts are forced read-only.
  void setReadOnly(bool read_only) { read_only_ = read_only; }

 protected:
  friend ArduinoSdFs CreateArduinoSdFs();

  ArduinoSdFs(uint8_t cs_pin = SS);

  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

  uint8_t cs_pin_;

  decltype(::SD)& sd_;
#if defined(ARDUINO_ARCH_RP2040)
  decltype(::SDFS)& sdfs_;
#endif
  bool read_only_;
};

#endif  // ESP32

/// Global Arduino SD filesystem instance.
extern ArduinoSdFs SD;

}  // namespace roo_io

#endif  // ARDUINO
