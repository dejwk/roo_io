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

bool ArduinoSpiSlaveDevice::init() {
  if (initialized_) return true;
  if (chip_select_ < 0 || frequency_ == 0 || mode_ > 3) return false;
  digitalWrite(chip_select_, HIGH);
  pinMode(chip_select_, OUTPUT);
  initialized_ = true;
  return true;
}

bool ArduinoSpiSlaveDevice::transfer(const uint8_t* tx, uint8_t* rx,
                                     size_t len) const {
  if (!initialized_ || len == 0 || (tx == nullptr && rx == nullptr))
    return false;
  static const uint8_t modes[] = {SPI_MODE0, SPI_MODE1, SPI_MODE2, SPI_MODE3};
  spi_.beginTransaction(SPISettings(frequency_, MSBFIRST, modes[mode_]));
  digitalWrite(chip_select_, LOW);
  for (size_t i = 0; i < len; ++i) {
    const uint8_t value = spi_.transfer(tx == nullptr ? 0 : tx[i]);
    if (rx != nullptr) rx[i] = value;
  }
  digitalWrite(chip_select_, HIGH);
  spi_.endTransaction();
  return true;
}
}  // namespace roo_io
#endif
