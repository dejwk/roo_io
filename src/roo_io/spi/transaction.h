#pragma once
#include <stddef.h>
#include <stdint.h>

namespace roo_io {

/// Owns one synchronous SPI bus reservation and active-low CS assertion.
/// The device and its application-initialized bus must outlive this scope.
/// A failed transfer leaves the scope open so a caller can attempt protocol
/// cleanup; check each return value. Destruction releases CS and the bus.
/// Serialize access externally and do not nest reservations on the same bus.
template <typename Device>
class BasicSpiTransaction {
 public:
  /// Reserves @p device and asserts CS; isOpen() reports acquisition failure.
  explicit BasicSpiTransaction(const Device& device)
      : device_(device.beginTransaction() ? &device : nullptr) {}

  /// Releases CS and the bus, including on early return from caller code.
  ~BasicSpiTransaction() { close(); }

  /// Transfers ownership without releasing the active reservation.
  BasicSpiTransaction(BasicSpiTransaction&& other) noexcept
      : device_(other.device_) {
    other.device_ = nullptr;
  }

  /// A reservation has one owner.
  BasicSpiTransaction(const BasicSpiTransaction&) = delete;

  /// A reservation cannot be copied or reassigned.
  BasicSpiTransaction& operator=(const BasicSpiTransaction&) = delete;

  /// A reservation cannot be reassigned.
  BasicSpiTransaction& operator=(BasicSpiTransaction&&) = delete;

  /// Reports whether acquisition succeeded and close() has not been called.
  bool isOpen() const { return device_ != nullptr; }

  /// Exchanges @p len bytes while keeping CS asserted. Uses the device's
  /// null-buffer and aliasing rules; false leaves the reservation open.
  bool transfer(const uint8_t* tx, uint8_t* rx, size_t len) const {
    return device_ != nullptr && device_->transferSelected(tx, rx, len);
  }

  /// Deasserts CS and releases the bus, even after transfer failure. Returns
  /// false if backend CS cleanup fails; always relinquishes ownership.
  /// Repeated calls are successful no-ops. The destructor ignores this result.
  bool close() {
    if (device_ == nullptr) return true;
    const Device* device = device_;
    device_ = nullptr;
    return device->endTransaction();
  }

 private:
  const Device* device_;
};
}  // namespace roo_io
