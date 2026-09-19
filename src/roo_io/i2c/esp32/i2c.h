#pragma once

#if defined(ESP_PLATFORM) && !defined(ARDUINO)
#include <memory>

#include "driver/i2c_master.h"
#include "roo_backport/byte.h"

namespace roo_io {

/// Copyable bus reference. Native-handle borrowing performs no allocation.
/// Port-based references share state so copies observe later initialization.
/// init() creates an owned bus kept alive by all copies and devices. Buses
/// obtained by port or native handle are borrowed and must outlive all devices.
/// Initialize before use; serialize initialization/destruction externally.
class Esp32I2cMasterBusHandle {
 public:
  /// Selects a port to initialize or to resolve to an externally created bus.
  Esp32I2cMasterBusHandle(i2c_port_num_t port = 0);

  /// Borrows an existing native bus without allocating or retaining ownership.
  /// Uses frequency for new devices; init() cannot create a bus through this
  /// view.
  explicit Esp32I2cMasterBusHandle(i2c_master_bus_handle_t handle,
                                   uint32_t frequency = 100000);

  /// Creates an owned bus; returns false on error or if already resolved.
  /// Enables internal pullups; frequency zero uses 100 kHz.
  bool init(int sda, int scl, uint32_t frequency = 100000);

 private:
  friend class Esp32I2cSlaveDevice;
  struct State;
  std::shared_ptr<State> state_;
  i2c_master_bus_handle_t borrowed_handle_ = nullptr;
  uint32_t borrowed_frequency_ = 100000;

  // Returns the device clock setting from owned/shared state or the borrowed
  // view.
  uint32_t frequency() const;

  // Resolves a borrowed port lazily, preserving failure for a later retry.
  i2c_master_bus_handle_t resolve() const;
};

/// Owns an IDF device registration and retains its bus reference.
/// Transfers before successful init() fail without aborting the application.
class Esp32I2cSlaveDevice {
 public:
  /// Binds an unshifted 7-bit address and finite, nonnegative timeout in ms.
  Esp32I2cSlaveDevice(Esp32I2cMasterBusHandle bus, uint8_t address,
                      int timeout_ms = 1000);

  /// Removes the device registration before releasing the bus reference.
  ~Esp32I2cSlaveDevice();

  /// Device registrations cannot be copied.
  Esp32I2cSlaveDevice(const Esp32I2cSlaveDevice&) = delete;

  /// Device registrations cannot be assigned.
  Esp32I2cSlaveDevice& operator=(const Esp32I2cSlaveDevice&) = delete;

  /// Registers the device. Idempotent; returns false on failure, allowing
  /// retry.
  bool init();

  /// Writes all bytes with a final STOP; returns false on error or timeout.
  /// The buffer must be non-null and len must be positive.
  bool transmit(const roo::byte* data, size_t len) const;

  /// Reads len bytes with a final STOP; returns len on success or zero on
  /// error. The buffer must be non-null and len must be positive.
  size_t receive(roo::byte* data, size_t len) const;

  /// Writes then reads with a repeated START and final STOP in one transaction.
  /// Returns true only if all bytes transfer. Buffers must be non-null and
  /// lengths positive. A failure can still partially update the device/buffer.
  bool transmitReceive(const roo::byte* tx, size_t tx_len, roo::byte* rx,
                       size_t rx_len) const;

 private:
  Esp32I2cMasterBusHandle bus_;
  uint8_t address_;
  int timeout_ms_;
  i2c_master_dev_handle_t device_ = nullptr;
};

}  // namespace roo_io
#endif
