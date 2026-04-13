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

bool SdSpiProbeCardArduino(SPIClass& spi, gpio_num_t cs_pin) {
  spi.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));

  // Ensure CS pin is configured as output.
  pinMode(cs_pin, OUTPUT);

  // Power-up: >=74 clocks with CS HIGH and MOSI HIGH.
  digitalWrite(cs_pin, HIGH);
  for (uint8_t i = 0; i < 10; i++) {
    spi.transfer(0xFF);
  }

  // CMD0 (GO_IDLE_STATE) with pre-computed CRC.
  // Retry up to 5 times: a freshly inserted card may need a few CMD0s to
  // transition from native mode to SPI mode.
  //  - 0xFF means no card (MISO pulled high, or no device driving it).
  //  - 0x00 means no card (another SPI device, e.g. ILI9488 with
  //    non-tri-state SDO, is driving MISO low).
  //  - 0x01 means card in idle state (present).
  //  - Anything else: retry.
  const uint8_t cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
  bool present = false;
  constexpr int kMaxRetries = 5;
  for (int attempt = 0; attempt < kMaxRetries; attempt++) {
    uint8_t token = SendCmd(spi, cs_pin, cmd0, sizeof(cmd0));
    if (token == 0xFF || token == 0x00) {
      break;
    }
    if (token == 0x01) {
      present = true;
      break;
    }
  }

  spi.endTransaction();
  return present;
}

bool SdSpiCheckStatusArduino(SPIClass& spi, gpio_num_t cs_pin) {
  spi.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));

  // CMD58 (READ_OCR), arg=0, CRC7=0x7E → CRC byte=(0x7E<<1)|1=0xFD.
  // The Arduino SD library enables CRC checking (CMD59), so a valid CRC is
  // required.
  const uint8_t cmd58[] = {0x40 | 58, 0x00, 0x00, 0x00, 0x00, 0xFD};
  digitalWrite(cs_pin, LOW);
  spi.writeBytes(cmd58, sizeof(cmd58));

  // Wait for R1 response.
  uint8_t r1 = 0xFF;
  for (int i = 0; i < 16; i++) {
    r1 = spi.transfer(0xFF);
    if (!(r1 & 0x80)) break;
  }

  // Read 4-byte OCR (even if R1 indicated an error, clock it out).
  uint32_t ocr = 0;
  for (int i = 0; i < 4; i++) {
    ocr = (ocr << 8) | spi.transfer(0xFF);
  }

  digitalWrite(cs_pin, HIGH);
  spi.transfer(0xFF);  // release DO
  spi.endTransaction();

  if (r1 & 0x80) return false;  // no valid response
  // A real card's OCR has voltage window + power-up bits set.
  // Absent card reads as 0x00000000 (MISO low) or 0xFFFFFFFF (MISO high).
  return ocr != 0 && ocr != 0xFFFFFFFF;
}

}  // namespace internal
}  // namespace roo_io

#endif  // ESP32 && ARDUINO
