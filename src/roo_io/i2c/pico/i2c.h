#pragma once
#if defined(PICO_RP2040) && PICO_RP2040
#include <hardware/i2c.h>

#include "roo_backport/byte.h"
namespace roo_io {
/// Borrows an initialized Pico SDK I2C controller, including its pin setup.
class PicoI2cMasterBusHandle {
 public:
  /// References @p bus without changing its configuration or lifetime.
  explicit PicoI2cMasterBusHandle(i2c_inst_t* bus = i2c0) : bus_(bus) {}

 private:
  friend class PicoI2cSlaveDevice;
  i2c_inst_t* bus_;
};

/// Synchronous SDK I2C device. Roo clients on each controller are serialized;
/// direct SDK clients must coordinate externally. Not callable from interrupts.
class PicoI2cSlaveDevice {
 public:
  /// Binds @p bus to unshifted @p address with @p timeout_us per SDK call.
  PicoI2cSlaveDevice(PicoI2cMasterBusHandle bus, uint8_t address,
                     uint32_t timeout_us = 1000000)
      : bus_(bus.bus_), address_(address), timeout_us_(timeout_us) {}
  /// Rejects an invalid controller, reserved/non-7-bit address, or zero timeout
  /// without I/O (valid addresses are 0x08 through 0x77).
  bool init();
  /// Sends a nonempty @p data buffer, ending in STOP; false on partial/error.
  bool transmit(const roo::byte* data, size_t len) const;
  /// Reads into @p data with STOP, returning the transferred prefix or zero.
  size_t receive(roo::byte* data, size_t len) const;
  /// Sends then receives under one lock, with repeated START and final STOP.
  /// False can partially update the device or receive buffer.
  bool transmitReceive(const roo::byte* tx, size_t tx_len, roo::byte* rx,
                       size_t rx_len) const;

 private:
  bool valid() const;
  i2c_inst_t* bus_;
  uint8_t address_;
  uint32_t timeout_us_;
};
}  // namespace roo_io
#endif
