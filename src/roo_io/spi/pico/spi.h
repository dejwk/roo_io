#pragma once
#if defined(PICO_RP2040) && PICO_RP2040
#include <hardware/spi.h>

#include "roo_io/spi/transaction.h"
namespace roo_io {
/// Borrows a Pico SDK controller initialized and routed by the application.
class PicoSpiMasterBusHandle {
 public:
  /// References @p bus without initializing or changing its pins.
  explicit PicoSpiMasterBusHandle(spi_inst_t* bus = spi0) : bus_(bus) {}

 private:
  friend class PicoSpiSlaveDevice;
  spi_inst_t* bus_;
};

/// Synchronous MSB-first, eight-bit SDK SPI device with active-low CS.
/// A scope serializes Roo devices on the controller until close/destruction.
/// Direct SDK clients require external coordination. Do not nest scopes on the
/// same controller or call from an ISR. The initialized controller must outlive
/// this device and all its scopes. Serialize use of the same device externally.
class PicoSpiSlaveDevice {
 public:
  /// Owns one controller reservation and CS assertion.
  using Transaction = BasicSpiTransaction<PicoSpiSlaveDevice>;
  /// Binds @p bus and @p chip_select with @p frequency Hz and @p mode (0-3).
  PicoSpiSlaveDevice(PicoSpiMasterBusHandle bus, int chip_select,
                     uint32_t frequency = 1000000, uint8_t mode = 0)
      : bus_(bus.bus_),
        chip_select_(chip_select),
        frequency_(frequency),
        mode_(mode) {}
  /// Creates an independent device referring to the same bus and settings.
  PicoSpiSlaveDevice(const PicoSpiSlaveDevice& other);
  /// Active identity cannot be reassigned.
  PicoSpiSlaveDevice& operator=(const PicoSpiSlaveDevice&) = delete;
  /// Configures CS idle-high; validates the borrowed controller and settings.
  bool init();
  /// Exchanges @p len bytes; null TX sends zeros, null RX discards data.
  /// Exact aliasing is supported; zero length/two null buffers are rejected.
  bool transfer(const uint8_t* tx, uint8_t* rx, size_t len) const;
  /// Opens a CS scope across multiple transfers; check isOpen().
  Transaction transaction() const { return Transaction(*this); }

 private:
  friend class BasicSpiTransaction<PicoSpiSlaveDevice>;
  bool beginTransaction() const;
  bool endTransaction() const;
  bool transferSelected(const uint8_t* tx, uint8_t* rx, size_t len) const;
  spi_inst_t* bus_;
  int chip_select_;
  uint32_t frequency_;
  uint8_t mode_;
  bool initialized_ = false;
  mutable bool active_ = false;
};
}  // namespace roo_io
#endif
