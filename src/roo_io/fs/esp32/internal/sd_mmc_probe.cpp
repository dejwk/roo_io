#include "roo_io/fs/esp32/internal/sd_mmc_probe.h"

#if defined(ESP_PLATFORM)

#include "soc/soc_caps.h"

#if SOC_SDMMC_HOST_SUPPORTED && !defined(ROO_TESTING)

#include <string.h>

#include "driver/sdmmc_defs.h"
#include "driver/sdmmc_host.h"

namespace roo_io {
namespace internal {

bool SdMmcProbe(int slot, const sdmmc_slot_config_t* slot_config) {
  esp_err_t ret = sdmmc_host_init();
  if (ret != ESP_OK) return false;

  ret = sdmmc_host_init_slot(slot, slot_config);
  if (ret != ESP_OK) {
    sdmmc_host_deinit();
    return false;
  }

  // sdmmc_host_init_slot sends 74 clock cycles at 400 kHz — sufficient to
  // bring the card into idle state.  No CMD0 needed.

  // Flush CMD8: absorbs any stale events left in the peripheral from a
  // previous session or from the init_slot clock cycles.
  //
  // timeout_ms is set generously so that timeout_ms / portTICK_PERIOD_MS
  // yields at least 1 tick even at low FreeRTOS tick rates (e.g. 100 Hz).
  // The actual wait is sub-millisecond: xQueueReceive returns as soon as
  // the hardware interrupt fires, not after the full timeout.
  sdmmc_command_t cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.opcode = SD_SEND_IF_COND;
  cmd.arg = (1 << 8) | 0x55;
  cmd.flags = SCF_CMD_BCR | SCF_RSP_R7;
  cmd.timeout_ms = 100;
  sdmmc_host_do_transaction(slot, &cmd);

  // Real CMD8 — SEND_IF_COND.  SD v2+ cards echo the check pattern in R7.
  memset(&cmd, 0, sizeof(cmd));
  cmd.opcode = SD_SEND_IF_COND;
  cmd.arg = (1 << 8) | 0xAA;  // 2.7–3.6 V, check pattern 0xAA
  cmd.flags = SCF_CMD_BCR | SCF_RSP_R7;
  cmd.timeout_ms = 100;
  ret = sdmmc_host_do_transaction(slot, &cmd);
  bool present =
      (ret == ESP_OK && (cmd.response[0] & 0xFFF) == ((1 << 8) | 0xAA));

  sdmmc_host_deinit();

  return present;
}

}  // namespace internal
}  // namespace roo_io

#endif  // SOC_SDMMC_HOST_SUPPORTED && !ROO_TESTING
#endif  // ESP_PLATFORM
