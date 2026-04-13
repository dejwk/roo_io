#pragma once

// Fast SPI-level card-presence probes.
//
// SdSpiProbeCardArduino(): Sends CMD0 (GO_IDLE_STATE) and checks for 0x01.
// Used when no filesystem is mounted.
//
// SdSpiCheckStatusArduino(): Sends CMD58 (READ_OCR) and checks for a valid
// OCR register.  Non-destructive — safe to call while a filesystem is mounted.
//
// Both complete in a few hundred microseconds, unlike the framework's
// disk_initialize / disk_status which can block for 500 ms – 1.8 s.

#if (defined ESP32 && defined ARDUINO)

#include <SPI.h>

#include "hal/gpio_types.h"

namespace roo_io {
namespace internal {

// Returns true if an SD card responded to CMD0 on the given SPI bus / CS pin.
// Use when NOT mounted (CMD0 resets the card).
bool SdSpiProbeCardArduino(SPIClass& spi, gpio_num_t cs_pin);

// Returns true if a mounted card responds to CMD58 (READ_OCR) with a valid
// OCR register.  Non-destructive — does not reset the card.
bool SdSpiCheckStatusArduino(SPIClass& spi, gpio_num_t cs_pin);

}  // namespace internal
}  // namespace roo_io

#endif  // ESP32 && ARDUINO
