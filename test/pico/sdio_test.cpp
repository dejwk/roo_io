#include "roo_io/fs/pico/arduino/sdio.h"

#include "gtest/gtest.h"
#include "stubs/SDFS.h"

SdioState sdio_state;

namespace {

class PicoSdioTest : public testing::Test {
 protected:
  void SetUp() override { sdio_state = SdioState(); }

  roo_io::PicoSdSdioFs fs_{3, 2, 4, 8.0f};
};

// Verifies a responding card with a later data failure remains present.
TEST_F(PicoSdioTest, UnmountedProbeDistinguishesMissingCardFromDataError) {
  sdio_state.init_error = SD_CARD_ERROR_CMD8;
  EXPECT_EQ(roo_io::Filesystem::kMediaAbsent, fs_.checkMediaPresence());
  EXPECT_EQ(roo_io::kNoMedia, fs_.mount().status());

  sdio_state.init_error = SD_CARD_ERROR_READ_CRC;
  EXPECT_EQ(roo_io::Filesystem::kMediaPresent, fs_.checkMediaPresence());
  EXPECT_EQ(roo_io::kGenericMountError, fs_.mount().status());
  EXPECT_EQ(0, sdio_state.volume_begins);
  EXPECT_EQ(sdio_state.card_begins, sdio_state.ends);
}

// Verifies ejection during a mount issues status commands without
// reinitializing the card, and unmount permits a fresh probe.
TEST_F(PicoSdioTest, MountedRemovalPreservesMountUntilClosed) {
  fs_.setMountingPolicy(roo_io::Filesystem::kMountReadOnly);
  roo_io::Mount mount = fs_.mount();
  ASSERT_TRUE(mount.ok());
  EXPECT_TRUE(mount.isReadOnly());
  EXPECT_EQ(roo_io::Filesystem::kMediaPresent, fs_.checkMediaPresence());
  sdio_state.responds = false;
  EXPECT_EQ(roo_io::Filesystem::kMediaAbsent, fs_.checkMediaPresence());
  EXPECT_TRUE(fs_.isMounted());
  EXPECT_EQ(1, sdio_state.card_begins);
  EXPECT_EQ(1, sdio_state.volume_begins);
  EXPECT_EQ(2, sdio_state.status_queries);

  mount.close();
  EXPECT_EQ(1, sdio_state.ends);
  sdio_state.init_error = SD_CARD_ERROR_CMD8;
  EXPECT_EQ(roo_io::Filesystem::kMediaAbsent, fs_.checkMediaPresence());
  sdio_state.init_error = 0;
  EXPECT_EQ(roo_io::Filesystem::kMediaPresent, fs_.checkMediaPresence());
  EXPECT_EQ(3, sdio_state.card_begins);
}

}  // namespace
