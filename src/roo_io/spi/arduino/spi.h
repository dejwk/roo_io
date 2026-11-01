#pragma once
#if defined(ARDUINO)
#include <SPI.h>
#include <stddef.h>
#include <stdint.h>

namespace roo_io {

/// Borrows an application-initialized SPI bus; the bus must outlive devices.
class ArduinoSpiMasterBusHandle {
 public:
  /// References @p spi without initializing it or changing its pins.
  ArduinoSpiMasterBusHandle(decltype(SPI)& spi = SPI) : spi_(spi) {}

 private:
  friend class ArduinoSpiSlaveDevice;
  decltype(SPI)& spi_;
};

/// Synchronous, MSB-first SPI device with active-low chip select.
/// The caller serializes access and initializes the borrowed bus. Arduino SPI
/// cannot report transfer errors; success means bytes were clocked by the API,
/// not that hardware acknowledged them. init() must precede transfers.
class ArduinoSpiSlaveDevice {
 public:
  /// Binds @p bus and @p chip_select at @p frequency Hz and SPI @p mode (0-3).
  ArduinoSpiSlaveDevice(ArduinoSpiMasterBusHandle bus, int chip_select,
                        uint32_t frequency = 1000000, uint8_t mode = 0);

  /// Configures CS idle-high; rejects negative pins, zero Hz, and invalid
  /// modes. Idempotent. Does not initialize the bus.
  bool init();

  /// Clocks @p len bytes under one CS assertion; sends zeros when @p tx is null
  /// and discards input when @p rx is null. Rejects zero length or two null
  /// buffers. Buffers may alias exactly; partial overlap is unsupported.
  bool transfer(const uint8_t* tx, uint8_t* rx, size_t len) const;

 private:
  decltype(SPI)& spi_;
  int chip_select_;
  uint32_t frequency_;
  uint8_t mode_;
  bool initialized_ = false;
};

}  // namespace roo_io
#endif
