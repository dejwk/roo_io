#pragma once

#if defined(ARDUINO)
#include <Wire.h>
#include <stddef.h>
#include <stdint.h>

#include "roo_backport/byte.h"

namespace roo_io {

/// Borrows Wire; the caller owns its lifetime and configures its timeout.
class ArduinoI2cMasterBusHandle {
 public:
  /// References the specified Wire instance without initializing it.
  ArduinoI2cMasterBusHandle(decltype(Wire)& wire = Wire) : wire_(wire) {}

  /// Initializes Wire using the core's default pins and settings.
  void init();

#if defined(ARDUINO_ARCH_ESP32)
  /// Initializes ESP32 Wire on the specified pins; returns false on failure.
  bool init(int sda, int scl, uint32_t frequency = 0);
#endif

 private:
  friend class ArduinoI2cSlaveDevice;
  decltype(Wire)& wire_;
};

/// Synchronous, 7-bit addressed device on a borrowed Wire bus.
/// Serialize access across tasks unless the core provides that guarantee.
class ArduinoI2cSlaveDevice {
 public:
  /// Binds a device to an unshifted 7-bit address without accessing hardware.
  ArduinoI2cSlaveDevice(ArduinoI2cMasterBusHandle bus, uint8_t address);

  /// Validates the address. The caller must initialize Wire separately.
  /// Optional on Arduino; provided for compatibility with the native backend.
  bool init();

  /// Writes all bytes with a final STOP; returns false on any failure.
  /// The buffer must be non-null and len must be positive.
  bool transmit(const roo::byte* data, size_t len) const;

  /// Reads at most len bytes with a final STOP; returns the number read.
  /// Returns zero for null buffers, zero length, or invalid addresses.
  size_t receive(roo::byte* data, size_t len) const;

  /// Writes then reads with a repeated START and a final STOP.
  /// Returns true only if all bytes transfer. Buffers must be non-null and
  /// lengths positive. A failure can still partially update the device/buffer.
  bool transmitReceive(const roo::byte* tx, size_t tx_len, roo::byte* rx,
                       size_t rx_len) const;

 private:
  // Copies available Wire bytes, stopping if read() reports an empty buffer.
  size_t readReceived(roo::byte* data, size_t len) const;
  decltype(Wire)& wire_;
  uint8_t address_;
};

}  // namespace roo_io
#endif
