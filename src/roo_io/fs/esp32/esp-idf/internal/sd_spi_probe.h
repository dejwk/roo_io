#pragma once

// Fast SPI-level card-presence probe for esp-idf SPI-mode SD.
//
// SdSpiProbeCard(): Sends CMD0 (GO_IDLE_STATE) and checks for 0x01.
// Used when no filesystem is mounted.
//
// Completes in a few hundred microseconds, unlike sdmmc_card_init which can
// block for hundreds of milliseconds.

#if defined(ESP_PLATFORM) && !defined(ARDUINO) && !defined(ROO_TESTING)

#include "hal/gpio_types.h"
#include "hal/spi_types.h"

namespace roo_io {
namespace internal {

// Returns true if an SD card responded to CMD0 on the given SPI host / CS pin.
// Use when NOT mounted (CMD0 resets the card).
bool SdSpiProbeCard(spi_host_device_t spi_host, gpio_num_t cs_pin);

}  // namespace internal
}  // namespace roo_io

#endif  // ESP_PLATFORM && !ARDUINO && !ROO_TESTING
