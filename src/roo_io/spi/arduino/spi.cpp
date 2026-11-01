#include "roo_io/spi/arduino/spi.h"
#if defined(ARDUINO)
#include <Arduino.h>

namespace roo_io {
ArduinoSpiSlaveDevice::ArduinoSpiSlaveDevice(ArduinoSpiMasterBusHandle bus,
                                             int chip_select,
                                             uint32_t frequency, uint8_t mode)
    : spi_(bus.spi_),
      chip_select_(chip_select),
      frequency_(frequency),
      mode_(mode) {}

ArduinoSpiSlaveDevice::ArduinoSpiSlaveDevice(const ArduinoSpiSlaveDevice& other)
    : ArduinoSpiSlaveDevice(ArduinoSpiMasterBusHandle(other.spi_),
                            other.chip_select_, other.frequency_, other.mode_) {
  initialized_ = other.initialized_;
}

bool ArduinoSpiSlaveDevice::init() {
  if (initialized_) return true;
  if (chip_select_ < 0 || frequency_ == 0 || mode_ > 3) return false;
  digitalWrite(chip_select_, HIGH);
  pinMode(chip_select_, OUTPUT);
  initialized_ = true;
  return true;
}

bool ArduinoSpiSlaveDevice::beginTransaction() const {
  if (!initialized_ || active_) return false;
  static const uint8_t modes[] = {SPI_MODE0, SPI_MODE1, SPI_MODE2, SPI_MODE3};
  spi_.beginTransaction(SPISettings(frequency_, MSBFIRST, modes[mode_]));
  digitalWrite(chip_select_, LOW);
  active_ = true;
  return true;
}

bool ArduinoSpiSlaveDevice::endTransaction() const {
  digitalWrite(chip_select_, HIGH);
  spi_.endTransaction();
  active_ = false;
  return true;
}

bool ArduinoSpiSlaveDevice::transferSelected(const uint8_t* tx, uint8_t* rx,
                                             size_t len) const {
  if (!active_ || len == 0 || (tx == nullptr && rx == nullptr)) return false;
  for (size_t i = 0; i < len; ++i) {
    const uint8_t value = spi_.transfer(tx == nullptr ? 0 : tx[i]);
    if (rx != nullptr) rx[i] = value;
  }
  return true;
}

bool ArduinoSpiSlaveDevice::transfer(const uint8_t* tx, uint8_t* rx,
                                     size_t len) const {
  if (len == 0 || (tx == nullptr && rx == nullptr)) return false;
  Transaction scope(*this);
  const bool success = scope.transfer(tx, rx, len);
  return scope.close() && success;
}
}  // namespace roo_io
#endif
