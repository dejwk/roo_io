#include "roo_io/fs/esp32/internal/sd_mmc_probe.h"

#include <vector>

#include "driver/sdmmc_defs.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gtest/gtest.h"

namespace {

// The host can restart without resetting a powered card. CMD8 responds only
// in idle state; after CMD0 it needs eight running clocks at 400 kHz.
struct ProbeState {
  bool present = true;
  bool idle = false;
  bool settled = false;
  bool bad_echo = false;
  bool clock_always_on = false;
  esp_err_t response_error = ESP_OK;
  esp_err_t reset_error = ESP_OK;
  esp_err_t clock_result = ESP_OK;
  esp_err_t init_result = ESP_OK;
  esp_err_t slot_result = ESP_OK;
  esp_err_t reset_result = ESP_OK;
  int deinitializations = 0;
  int resets = 0;
  int slot = -1;
  const sdmmc_slot_config_t* slot_config = nullptr;
  std::vector<int> commands;
  std::vector<uint32_t> delays_us;
  std::vector<TickType_t> task_delays;
};

ProbeState state;

class SdMmcProbeTest : public testing::Test {
 protected:
  void SetUp() override { state = ProbeState(); }

  bool probe() {
    return roo_io::internal::SdMmcProbe(SDMMC_HOST_SLOT_1, &config_);
  }

  sdmmc_slot_config_t config_ = SDMMC_SLOT_CONFIG_DEFAULT();
};

// Verifies an idle card takes the fast path without a reset or settling wait.
TEST_F(SdMmcProbeTest, IdleCardNeedsNoReset) {
  state.idle = true;
  state.settled = true;
  EXPECT_TRUE(probe());
  EXPECT_EQ(0, state.resets);
  EXPECT_TRUE(state.delays_us.empty());
  EXPECT_TRUE(state.task_delays.empty());
  EXPECT_EQ(1, state.deinitializations);
  EXPECT_EQ(&config_, state.slot_config);
  EXPECT_EQ(SDMMC_HOST_SLOT_1, state.slot);
}

// Verifies a card retaining its operating state across reboot is reset and
// given eight running clock cycles before the successful CMD8 retry.
TEST_F(SdMmcProbeTest, WarmRebootResetsPoweredCard) {
  EXPECT_TRUE(probe());
  EXPECT_EQ(1, state.resets);
  ASSERT_EQ(1u, state.delays_us.size());
  EXPECT_GE(state.delays_us[0], 20u);
  EXPECT_LT(state.delays_us[0], 1000u);
  EXPECT_TRUE(state.task_delays.empty());
  EXPECT_EQ(SD_SEND_IF_COND, state.commands.back());
  EXPECT_EQ(1, state.deinitializations);
}

// Verifies recovery is available on later sessions, not just once per boot.
TEST_F(SdMmcProbeTest, LaterUnmountedSessionCanAlsoRecover) {
  ASSERT_TRUE(probe());
  // A mount used the card, then unmounted without removing card power.
  state.idle = false;
  state.settled = false;
  EXPECT_TRUE(probe());
  EXPECT_EQ(2, state.resets);
  EXPECT_EQ(2, state.deinitializations);
}

// Verifies CMD0's lack of a response cannot mistake an empty slot for a card.
TEST_F(SdMmcProbeTest, EmptySlotRemainsAbsent) {
  state.present = false;
  EXPECT_FALSE(probe());
  EXPECT_TRUE(state.task_delays.empty());
  ASSERT_EQ(1u, state.delays_us.size());
  EXPECT_LT(state.delays_us[0], 1000u);
  EXPECT_EQ(1, state.deinitializations);
}

// Verifies a responding card must still echo the expected voltage and pattern.
TEST_F(SdMmcProbeTest, InvalidEchoRemainsRejected) {
  state.idle = true;
  state.settled = true;
  state.bad_echo = true;
  EXPECT_FALSE(probe());
  EXPECT_EQ(1, state.deinitializations);
}

// Verifies a stale matching echo is rejected when IDF reports a response error
// through command.error despite returning ESP_OK from the transaction.
TEST_F(SdMmcProbeTest, ResponseErrorRejectsMatchingEcho) {
  state.idle = true;
  state.settled = true;
  state.response_error = ESP_ERR_INVALID_CRC;
  EXPECT_FALSE(probe());
  EXPECT_EQ(1, state.resets);
  EXPECT_EQ(1, state.deinitializations);
}

// Verifies failure to keep the clock running aborts before issuing a reset.
TEST_F(SdMmcProbeTest, ClockFailureReleasesHost) {
  state.clock_result = ESP_FAIL;
  EXPECT_FALSE(probe());
  EXPECT_EQ(0, state.resets);
  EXPECT_TRUE(state.delays_us.empty());
  EXPECT_EQ(1, state.deinitializations);
}

// Verifies an IDF command error also prevents treating CMD0 as successful.
TEST_F(SdMmcProbeTest, ResetCommandErrorReleasesHost) {
  state.reset_error = ESP_FAIL;
  EXPECT_FALSE(probe());
  ASSERT_FALSE(state.commands.empty());
  EXPECT_EQ(MMC_GO_IDLE_STATE, state.commands.back());
  EXPECT_TRUE(state.delays_us.empty());
  EXPECT_EQ(1, state.deinitializations);
}

// Verifies reset failure releases the acquired host and does not probe again.
TEST_F(SdMmcProbeTest, ResetFailureReleasesHost) {
  state.reset_result = ESP_FAIL;
  EXPECT_FALSE(probe());
  ASSERT_FALSE(state.commands.empty());
  EXPECT_EQ(MMC_GO_IDLE_STATE, state.commands.back());
  EXPECT_TRUE(state.delays_us.empty());
  EXPECT_EQ(1, state.deinitializations);
}

// Verifies a host already in use is neither reset nor deinitialized.
TEST_F(SdMmcProbeTest, UnavailableHostIsLeftAlone) {
  state.init_result = ESP_ERR_INVALID_STATE;
  EXPECT_FALSE(probe());
  EXPECT_TRUE(state.commands.empty());
  EXPECT_EQ(0, state.deinitializations);
}

// Verifies slot configuration failure releases the host without card commands.
TEST_F(SdMmcProbeTest, SlotFailureReleasesHost) {
  state.slot_result = ESP_FAIL;
  EXPECT_FALSE(probe());
  EXPECT_TRUE(state.commands.empty());
  EXPECT_EQ(1, state.deinitializations);
}

}  // namespace

extern "C" esp_err_t sdmmc_host_init() { return state.init_result; }

extern "C" esp_err_t sdmmc_host_init_slot(
    int slot, const sdmmc_slot_config_t* slot_config) {
  state.slot = slot;
  state.slot_config = slot_config;
  state.clock_always_on = false;
  return state.slot_result;
}

extern "C" esp_err_t sdmmc_host_set_cclk_always_on(int slot, bool always_on) {
  EXPECT_EQ(state.slot, slot);
  if (state.clock_result != ESP_OK) return state.clock_result;
  state.clock_always_on = always_on;
  return ESP_OK;
}

extern "C" esp_err_t sdmmc_host_do_transaction(int slot,
                                               sdmmc_command_t* command) {
  EXPECT_EQ(state.slot, slot);
  state.commands.push_back(command->opcode);
  if (command->opcode == MMC_GO_IDLE_STATE) {
    ++state.resets;
    EXPECT_EQ(SCF_CMD_BC | SCF_RSP_R0, command->flags);
    EXPECT_EQ(0u, command->arg);
    if (state.reset_result != ESP_OK) return state.reset_result;
    command->error = state.reset_error;
    state.idle = state.present;
    state.settled = false;
    return ESP_OK;
  }
  EXPECT_EQ(SD_SEND_IF_COND, command->opcode);
  EXPECT_EQ(SCF_CMD_BCR | SCF_RSP_R7, command->flags);
  if (!state.present || !state.idle || !state.settled) {
    command->error = ESP_ERR_TIMEOUT;
    return ESP_OK;
  }
  command->error = state.response_error;
  command->response[0] = state.bad_echo ? 0 : command->arg;
  return ESP_OK;
}

extern "C" esp_err_t sdmmc_host_deinit() {
  ++state.deinitializations;
  state.clock_always_on = false;
  return ESP_OK;
}

extern "C" void esp_rom_delay_us(uint32_t us) {
  state.delays_us.push_back(us);
  // Waiting with a gated clock does not supply the card's reset clocks.
  state.settled = state.clock_always_on && us >= 20;
}

extern "C" void vTaskDelay(TickType_t ticks) {
  state.task_delays.push_back(ticks);
  ADD_FAILURE() << "Presence probing must not sleep for scheduler ticks";
}
