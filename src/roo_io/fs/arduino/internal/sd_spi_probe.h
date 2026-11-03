#pragma once

#if defined(ARDUINO)
#include <SPI.h>
#include <stdint.h>

namespace roo_io {
namespace internal {

// Probes an initialized SPI controller using CMD0 at 400 kHz. This resets a
// responding card, so call only while its filesystem is unmounted.
bool SdSpiProbeCard(decltype(::SPI)& spi, uint8_t cs_pin);

// Checks an initialized card with CMD58 without resetting or mounting it.
// Both probes use bounded response polling, leave CS high and end the SPI
// transaction. Callers serialize access to the card and shared SPI controller.
bool SdSpiCheckStatus(decltype(::SPI)& spi, uint8_t cs_pin);

}  // namespace internal
}  // namespace roo_io
#endif
