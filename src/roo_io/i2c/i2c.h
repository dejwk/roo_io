#pragma once

// Synchronous, addressed I2C transactions. Arduino takes precedence when both
// framework macros are defined. Native ESP-IDF is enabled without Arduino.
#if defined(ARDUINO)
#include "roo_io/i2c/arduino/i2c.h"
namespace roo_io {
using I2cMasterBusHandle = ArduinoI2cMasterBusHandle;
using I2cSlaveDevice = ArduinoI2cSlaveDevice;
}  // namespace roo_io
#elif defined(ESP_PLATFORM)
#include "roo_io/i2c/esp32/i2c.h"
namespace roo_io {
using I2cMasterBusHandle = Esp32I2cMasterBusHandle;
using I2cSlaveDevice = Esp32I2cSlaveDevice;
}  // namespace roo_io
#elif defined(PICO_RP2040) && PICO_RP2040
#include "roo_io/i2c/pico/i2c.h"
namespace roo_io {
using I2cMasterBusHandle = PicoI2cMasterBusHandle;
using I2cSlaveDevice = PicoI2cSlaveDevice;
}  // namespace roo_io
#endif
