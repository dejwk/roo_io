#include "roo_io/fs/esp32/arduino/internal/sd_spi_probe.h"

#if defined(ESP32) && defined(ARDUINO)
#include "roo_io/fs/arduino/internal/sd_spi_probe.h"

namespace roo_io {
namespace internal {

bool SdSpiProbeCardArduino(SPIClass& spi, gpio_num_t cs_pin) {
  return SdSpiProbeCard(spi, static_cast<uint8_t>(cs_pin));
}

bool SdSpiCheckStatusArduino(SPIClass& spi, gpio_num_t cs_pin) {
  return SdSpiCheckStatus(spi, static_cast<uint8_t>(cs_pin));
}

}  // namespace internal
}  // namespace roo_io
#endif
