#pragma once

// Fast SDMMC-bus card-presence probe.
//
// SdMmcProbe() initialises the SDMMC host, sends a single CMD8, and tears
// the host back down.  The full cycle takes ~2–3 ms.  The host is always
// deinitialised on return so that esp_vfs_fat_sdmmc_mount() can call
// sdmmc_host_init() without conflict.

#if defined(ESP_PLATFORM)

#include "soc/soc_caps.h"

#if SOC_SDMMC_HOST_SUPPORTED && !defined(ROO_TESTING)

#include "driver/sdmmc_host.h"

namespace roo_io {
namespace internal {

// Probe for an SD card on the given SDMMC slot.  Returns true if a card
// responded to CMD8 with a valid echo pattern.
bool SdMmcProbe(int slot, const sdmmc_slot_config_t* slot_config);

}  // namespace internal
}  // namespace roo_io

#endif  // SOC_SDMMC_HOST_SUPPORTED && !ROO_TESTING
#endif  // ESP_PLATFORM
