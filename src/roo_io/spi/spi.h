#pragma once

// Arduino takes precedence when both framework macros are defined.
#if defined(ARDUINO)
#include "roo_io/spi/arduino/spi.h"
namespace roo_io {
using SpiMasterBusHandle = ArduinoSpiMasterBusHandle;
using SpiSlaveDevice = ArduinoSpiSlaveDevice;
using SpiTransaction = SpiSlaveDevice::Transaction;
}  // namespace roo_io
#elif defined(ESP_PLATFORM)
#include "roo_io/spi/esp32/spi.h"
namespace roo_io {
using SpiMasterBusHandle = Esp32SpiMasterBusHandle;
using SpiSlaveDevice = Esp32SpiSlaveDevice;
using SpiTransaction = SpiSlaveDevice::Transaction;
}  // namespace roo_io
#endif
