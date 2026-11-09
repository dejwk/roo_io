#include "roo_io/fs/esp32/internal/sd_mmc_probe.h"

#if defined(ESP_PLATFORM)

#include "soc/soc_caps.h"

#if SOC_SDMMC_HOST_SUPPORTED && !defined(ROO_TESTING)

#include "driver/sdmmc_defs.h"
#include "driver/sdmmc_host.h"
#include "esp_rom_sys.h"

namespace roo_io {
namespace internal {
namespace {

// Checks whether an idle SD v2+ card echoes the voltage and check pattern.
bool SendInterfaceCondition(int slot, uint8_t pattern) {
  sdmmc_command_t cmd = {};
  cmd.opcode = SD_SEND_IF_COND;
  cmd.arg = (1 << 8) | pattern;  // 2.7–3.6 V.
  cmd.flags = SCF_CMD_BCR | SCF_RSP_R7;
  // Software guard for missing driver events. Normal no-response detection
  // uses the controller timeout (255 clocks, about 0.64 ms at 400 kHz).
  cmd.timeout_ms = 100;
  esp_err_t ret = sdmmc_host_do_transaction(slot, &cmd);
  return ret == ESP_OK && cmd.error == ESP_OK &&
         (cmd.response[0] & 0xFFF) == cmd.arg;
}

}  // namespace

bool SdMmcProbe(int slot, const sdmmc_slot_config_t* slot_config) {
  esp_err_t ret = sdmmc_host_init();
  if (ret != ESP_OK) return false;

  ret = sdmmc_host_init_slot(slot, slot_config);
  if (ret != ESP_OK) {
    sdmmc_host_deinit();
    return false;
  }

  // Flush CMD8 absorbs stale events from host/slot initialization before the
  // actual probe. An already-idle card needs no reset or settling delay.
  SendInterfaceCondition(slot, 0x55);
  bool present = SendInterfaceCondition(slot, 0xAA);
  if (!present) {
    // A powered card can retain its operating state across host reboot or
    // unmount. Startup clocks alone do not reset it to idle for CMD8.
    // Slot initialization selects 400 kHz but allows clock gating between
    // commands. Keep it running so the post-reset wait supplies real clocks.
    ret = sdmmc_host_set_cclk_always_on(slot, true);
    if (ret != ESP_OK) {
      sdmmc_host_deinit();
      return false;
    }
    sdmmc_command_t cmd = {};
    cmd.opcode = MMC_GO_IDLE_STATE;
    cmd.flags = SCF_CMD_BC | SCF_RSP_R0;
    cmd.timeout_ms = 100;
    ret = sdmmc_host_do_transaction(slot, &cmd);
    if (ret == ESP_OK && cmd.error == ESP_OK) {
      // SD Physical Layer spec 4.3.10.1: allow eight clocks after CMD0 for
      // return-to-default switching. At 400 kHz this is 20 us, not 20 ms.
      esp_rom_delay_us(20);
      present = SendInterfaceCondition(slot, 0xAA);
    }
  }

  sdmmc_host_deinit();
  return present;
}

}  // namespace internal
}  // namespace roo_io

#endif  // SOC_SDMMC_HOST_SUPPORTED && !ROO_TESTING
#endif  // ESP_PLATFORM
