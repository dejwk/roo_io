#pragma once

#if defined(ARDUINO) && defined(ARDUINO_ARCH_RP2040)
#include <SD.h>
#include <SPI.h>

#include "roo_io/fs/filesystem.h"

namespace roo_io {

/// Wraps Arduino-Pico's global SD/SDFS backend with automatic card detection.
/// Unmounted checks use a reset probe; mounted checks preserve card state.
/// Serialize filesystem access and sharing of the borrowed SPI controller
/// externally. Configure SPI,
/// CS and frequency while unmounted; retain the controller through all mounts.
class PicoSdSpiFs : public Filesystem {
 public:
  /// Releases cached mounts before the concrete filesystem is destroyed.
  ~PicoSdSpiFs() override {
    if (isMounted()) forceUnmount();
  }

  /// Detects media without mounting it; may initialize the SPI controller.
  MediaPresence checkMediaPresence() override;

  /// Selects the card's chip-select GPIO while unmounted.
  void setCsPin(uint8_t cs_pin) { cs_pin_ = cs_pin; }

  /// Borrows @p spi, using its configured pins, while unmounted.
  void setSPI(decltype(::SPI)& spi) { spi_ = &spi; }

  /// Sets maximum mount frequency in Hz while unmounted (default 4 MHz).
  void setFrequency(uint32_t frequency) { frequency_ = frequency; }

  /// Returns the configured maximum mount frequency in Hz.
  uint32_t frequency() const { return frequency_; }

  /// Returns whether the backend forces new mounts read-only.
  bool readOnly() const { return read_only_; }

  /// Forces subsequent backend mounts read-only when @p read_only is true.
  void setReadOnly(bool read_only) { read_only_ = read_only; }

 protected:
  friend PicoSdSpiFs CreatePicoSdSpiFs();

  /// Creates the singleton wrapper over core SD/SDFS, borrowing @p spi.
  PicoSdSpiFs(uint8_t cs_pin = SS, decltype(::SPI)& spi = ::SPI,
              uint32_t frequency = 4000000);

  /// Mounts the core SDFS backend after checking card presence.
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  /// Unmounts SDFS without ending the application-owned SPI controller.
  void unmountImpl() override;

 private:
  decltype(::SPI)* spi_;
  uint32_t frequency_;
  uint8_t cs_pin_;
  bool read_only_ = false;
};

/// Global Pico Arduino SD filesystem wrapper.
extern PicoSdSpiFs SD;

}  // namespace roo_io
#endif
