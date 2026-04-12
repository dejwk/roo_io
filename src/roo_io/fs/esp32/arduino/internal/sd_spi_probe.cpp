#include "roo_io/fs/esp32/arduino/internal/sd_spi_probe.h"

#if (defined ESP32 && defined ARDUINO)

#include <Arduino.h>

namespace roo_io {
namespace internal {
namespace {

// Select card, send a raw command, read R1 response, deselect.
// Returns the R1 byte, or 0xFF if no response within `max_read` bytes.
uint8_t SendCmd(SPIClass& spi, gpio_num_t cs_pin, const uint8_t* cmd,
                size_t cmd_len, int max_read = 16) {
  digitalWrite(cs_pin, LOW);
  spi.writeBytes(cmd, cmd_len);

  uint8_t token = 0xFF;
  for (int i = 0; i < max_read; i++) {
    token = spi.transfer(0xFF);
    if (!(token & 0x80)) break;
  }

  digitalWrite(cs_pin, HIGH);
  spi.transfer(0xFF);  // release DO
  return token;
}

}  // namespace

bool SdSpiProbeCard(SPIClass& spi, gpio_num_t cs_pin) {
  spi.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));

  // Ensure CS pin is configured as output.
  pinMode(cs_pin, OUTPUT);

  // Power-up: >=74 clocks with CS HIGH and MOSI HIGH.
  digitalWrite(cs_pin, HIGH);
  for (uint8_t i = 0; i < 10; i++) {
    spi.transfer(0xFF);
  }

  // CMD0 (GO_IDLE_STATE) with pre-computed CRC.
  const uint8_t cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
  uint8_t token = SendCmd(spi, cs_pin, cmd0, sizeof(cmd0));

  spi.endTransaction();
  return token == 0x01;  // exactly the idle-state R1
}

bool SdSpiCheckStatus(SPIClass& spi, gpio_num_t cs_pin) {
  spi.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));

  // CMD13 (SEND_STATUS), arg=0, dummy CRC.
  const uint8_t cmd13[] = {0x40 | 13, 0x00, 0x00, 0x00, 0x00, 0x01};
  uint8_t token = SendCmd(spi, cs_pin, cmd13, sizeof(cmd13));

  if (!(token & 0x80)) {
    // Read and discard the second status byte (R2 response).
    spi.transfer(0xFF);
  }

  spi.endTransaction();
  // Any valid R1 (bit 7 clear, and not 0x00 which is a floating MISO) means
  // the card is present.
  return !(token & 0x80) && token != 0x00;
}

}  // namespace internal
}  // namespace roo_io

#endif  // ESP32 && ARDUINO
