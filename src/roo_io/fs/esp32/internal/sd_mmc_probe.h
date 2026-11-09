#pragma once

#if defined(ESP_PLATFORM)

#include "soc/soc_caps.h"

#if SOC_SDMMC_HOST_SUPPORTED && !defined(ROO_TESTING)

#include "driver/sdmmc_host.h"

namespace roo_io {
namespace internal {

/// Probes an unmounted SD v2+ card for a valid CMD8 voltage/pattern echo.
/// Initializes the host with @p slot_config on @p slot. If CMD8 fails, resets
/// the card with native-mode CMD0, supplies at least eight clocks at 400 kHz
/// (20 us), and retries. An already-idle card needs no reset or settling delay.
/// Driver timeouts still apply; this is not a total probe time bound.
/// Releases the host acquired by this call
/// before returning, including on failure, so normal mounting can initialize
/// it again. Returns false if initialization or the final probe fails.
/// The SDMMC host must be unused and no card may be mounted: the fallback
/// resets card protocol state. Callers must serialize access to the host.
bool SdMmcProbe(int slot, const sdmmc_slot_config_t* slot_config);

}  // namespace internal
}  // namespace roo_io

#endif  // SOC_SDMMC_HOST_SUPPORTED && !ROO_TESTING
#endif  // ESP_PLATFORM
