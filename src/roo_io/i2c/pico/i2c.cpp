#include "roo_io/i2c/pico/i2c.h"
#if defined(PICO_RP2040) && PICO_RP2040
#include <climits>

#include "roo_threads/mutex.h"
namespace roo_io {
namespace {
roo::mutex locks[2];
}
bool PicoI2cSlaveDevice::valid() const {
  return (bus_ == i2c0 || bus_ == i2c1) && address_ >= 0x08 &&
         address_ <= 0x77 && timeout_us_ != 0;
}
bool PicoI2cSlaveDevice::init() { return valid(); }
bool PicoI2cSlaveDevice::transmit(const roo::byte* data, size_t len) const {
  if (!valid() || data == nullptr || len == 0 || len > INT_MAX) return false;
  roo::lock_guard<roo::mutex> lock(locks[bus_ == i2c0 ? 0 : 1]);
  return i2c_write_timeout_us(bus_, address_,
                              reinterpret_cast<const uint8_t*>(data), len,
                              false, timeout_us_) == static_cast<int>(len);
}
size_t PicoI2cSlaveDevice::receive(roo::byte* data, size_t len) const {
  if (!valid() || data == nullptr || len == 0 || len > INT_MAX) return 0;
  roo::lock_guard<roo::mutex> lock(locks[bus_ == i2c0 ? 0 : 1]);
  const int result =
      i2c_read_timeout_us(bus_, address_, reinterpret_cast<uint8_t*>(data), len,
                          false, timeout_us_);
  return result > 0 ? result : 0;
}
bool PicoI2cSlaveDevice::transmitReceive(const roo::byte* tx, size_t tx_len,
                                         roo::byte* rx, size_t rx_len) const {
  if (!valid() || tx == nullptr || rx == nullptr || tx_len == 0 ||
      rx_len == 0 || tx_len > INT_MAX || rx_len > INT_MAX)
    return false;
  roo::lock_guard<roo::mutex> lock(locks[bus_ == i2c0 ? 0 : 1]);
  const int sent =
      i2c_write_timeout_us(bus_, address_, reinterpret_cast<const uint8_t*>(tx),
                           tx_len, true, timeout_us_);
  if (sent != static_cast<int>(tx_len)) return false;
  return i2c_read_timeout_us(bus_, address_, reinterpret_cast<uint8_t*>(rx),
                             rx_len, false,
                             timeout_us_) == static_cast<int>(rx_len);
}
}  // namespace roo_io
#endif
