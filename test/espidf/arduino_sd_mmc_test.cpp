#include "gtest/gtest.h"
#include "roo_io/fs/esp32/arduino/sdmmc.h"
#include "roo_io/fs/esp32/internal/sd_mmc_probe.h"
#include "sdmmc_cmd.h"

namespace {

struct ProbeState {
  bool present = true;
  int unmounted_probes = 0;
  int status_queries = 0;
  sdmmc_card_t card{};
};

ProbeState state;

class MountedCard : public fs::SDMMCFS {
 public:
  static void Attach() { ::SD_MMC.*& MountedCard::_card = &state.card; }
};

// Verifies the mounted card is queried directly even when Arduino's cached
// mount still exists. Only unmounted checks may reset the card.
TEST(ArduinoSdMmcTest, RemovalUsesLiveStatusWithoutUnmounting) {
  state = ProbeState();
  roo_io::Mount mount = roo_io::SD_MMC.mount();
  ASSERT_TRUE(mount.ok());
  MountedCard::Attach();

  EXPECT_EQ(roo_io::Filesystem::kMediaPresent,
            roo_io::SD_MMC.checkMediaPresence());
  state.present = false;
  EXPECT_EQ(roo_io::Filesystem::kMediaAbsent,
            roo_io::SD_MMC.checkMediaPresence());
  EXPECT_TRUE(roo_io::SD_MMC.isMounted());
  EXPECT_EQ(2, state.status_queries);
  EXPECT_EQ(1, state.unmounted_probes);

  mount.close();
  EXPECT_EQ(roo_io::Filesystem::kMediaAbsent,
            roo_io::SD_MMC.checkMediaPresence());
  state.present = true;
  EXPECT_EQ(roo_io::Filesystem::kMediaPresent,
            roo_io::SD_MMC.checkMediaPresence());
  EXPECT_EQ(3, state.unmounted_probes);
  EXPECT_EQ(2, state.status_queries);
}

}  // namespace

namespace roo_io::internal {

bool SdMmcProbe(int slot, const sdmmc_slot_config_t* config) {
  ++state.unmounted_probes;
  return state.present;
}

}  // namespace roo_io::internal

extern "C" esp_err_t sdmmc_get_status(sdmmc_card_t* card) {
  EXPECT_EQ(&state.card, card);
  ++state.status_queries;
  return state.present ? ESP_OK : ESP_ERR_TIMEOUT;
}
