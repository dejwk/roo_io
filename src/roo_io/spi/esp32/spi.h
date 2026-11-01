#pragma once
#if defined(ESP_PLATFORM) && !defined(ARDUINO)
#include <stddef.h>
#include <stdint.h>

#include "driver/spi_master.h"

namespace roo_io {

/// Borrows an initialized ESP-IDF SPI host; the application owns its lifetime.
class Esp32SpiMasterBusHandle {
 public:
  /// Selects @p host, initialized separately with spi_bus_initialize().
  Esp32SpiMasterBusHandle(spi_host_device_t host = SPI2_HOST) : host_(host) {}

 private:
  friend class Esp32SpiSlaveDevice;
  spi_host_device_t host_;
};

/// Owns an IDF device registration on a borrowed SPI host.
/// Serialize calls externally. Destruction removes the registration; the host
/// must remain initialized until all devices are destroyed.
class Esp32SpiSlaveDevice {
 public:
  /// Binds @p bus and active-low @p chip_select at @p frequency Hz, MSB first,
  /// using SPI @p mode (0-3). The maximum transfer size is set by the host.
  Esp32SpiSlaveDevice(Esp32SpiMasterBusHandle bus, int chip_select,
                      uint32_t frequency = 1000000, uint8_t mode = 0);

  /// Removes an owned registration, if any.
  ~Esp32SpiSlaveDevice();

  /// Registrations cannot be copied.
  Esp32SpiSlaveDevice(const Esp32SpiSlaveDevice&) = delete;

  /// Registrations cannot be assigned.
  Esp32SpiSlaveDevice& operator=(const Esp32SpiSlaveDevice&) = delete;

  /// Registers the device; idempotent and retryable after failure.
  bool init();

  /// Clocks @p len bytes under one CS assertion. Null @p tx disables the MOSI
  /// phase (dummy levels unspecified); null @p rx discards input. Rejects zero
  /// length or two null buffers; returns false on driver errors. Uses
  /// synchronous IDF polling (no finite timeout). Buffers may alias exactly;
  /// partial overlap is unsupported.
  bool transfer(const uint8_t* tx, uint8_t* rx, size_t len) const;

 private:
  spi_host_device_t host_;
  int chip_select_;
  uint32_t frequency_;
  uint8_t mode_;
  spi_device_handle_t device_ = nullptr;
};

}  // namespace roo_io
#endif
