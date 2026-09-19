#include "roo_io/i2c/arduino/i2c.h"

#if defined(ARDUINO)
namespace roo_io {
void ArduinoI2cMasterBusHandle::init() { wire_.begin(); }
#if defined(ARDUINO_ARCH_ESP32)
bool ArduinoI2cMasterBusHandle::init(int sda, int scl, uint32_t frequency) {
  return wire_.begin(sda, scl, frequency);
}
#endif
ArduinoI2cSlaveDevice::ArduinoI2cSlaveDevice(ArduinoI2cMasterBusHandle bus,
                                             uint8_t address)
    : wire_(bus.wire_), address_(address) {}

bool ArduinoI2cSlaveDevice::init() { return address_ <= 0x7f; }

bool ArduinoI2cSlaveDevice::transmit(const roo::byte* data, size_t len) const {
  if (address_ > 0x7f || data == nullptr || len == 0) {
    return false;
  }
  wire_.beginTransmission(address_);
  const size_t written =
      wire_.write(reinterpret_cast<const uint8_t*>(data), len);
  // Finish even a partial enqueue so Wire releases its transaction state.
  const uint8_t status = wire_.endTransmission();
  return written == len && status == 0;
}

size_t ArduinoI2cSlaveDevice::readReceived(roo::byte* data, size_t len) const {
  size_t count = 0;
  for (; count < len; ++count) {
    const int value = wire_.read();
    if (value < 0) {
      break;
    }
    data[count] = static_cast<roo::byte>(value);
  }
  return count;
}

size_t ArduinoI2cSlaveDevice::receive(roo::byte* data, size_t len) const {
  if (address_ > 0x7f || data == nullptr || len == 0) {
    return 0;
  }
  const size_t received = wire_.requestFrom(address_, len);
  return readReceived(data, received < len ? received : len);
}

bool ArduinoI2cSlaveDevice::transmitReceive(const roo::byte* tx, size_t tx_len,
                                            roo::byte* rx,
                                            size_t rx_len) const {
  if (address_ > 0x7f || tx == nullptr || rx == nullptr || tx_len == 0 ||
      rx_len == 0) {
    return false;
  }
  wire_.beginTransmission(address_);
  const size_t written =
      wire_.write(reinterpret_cast<const uint8_t*>(tx), tx_len);
  if (written != tx_len) {
    wire_.endTransmission();
    return false;
  }
  if (wire_.endTransmission(false) != 0) {
    return false;
  }
  const size_t received = wire_.requestFrom(address_, rx_len);
  if (received != rx_len) {
    return false;
  }
  return readReceived(rx, rx_len) == rx_len;
}
}  // namespace roo_io
#endif
