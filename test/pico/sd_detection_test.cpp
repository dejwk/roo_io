#include <SD.h>
#include <SPI.h>

#include <type_traits>

#include "gtest/gtest.h"
#include "roo_io/fs/arduino/sdfs.h"
#include "roo_io/fs/pico/arduino/sdspi.h"
#include "roo_testing/devices/storage/sd_spi/sd_spi.h"
#include "roo_testing/frameworks/arduino_pico_support/filesystem.h"
#include "roo_testing/microcontrollers/rp2040/fake_pico.h"

namespace {
static_assert(std::is_same<roo_io::ArduinoSdFs, roo_io::PicoSdSpiFs>::value,
              "The Arduino compatibility header must select Pico SD");

struct Wiring {
  Wiring() {
    FakePico().attachSpiDevice(cards[0], 18, 16, 19);
    FakePico().attachSpiDevice(cards[1], 14, 12, 15);
    FakePico().attachOutput(17, cards[0].cs());
    FakePico().attachOutput(13, cards[1].cs());
  }
  FakeSdSpiCard cards[2];
} wiring;

class PicoSdDetection : public testing::Test {
 protected:
  void SetUp() override {
    roo_io::SD.forceUnmount();
    roo_io::SD.setSPI(SPI);
    roo_io::SD.setCsPin(17);
    roo_io::SD.setFrequency(4000000);
    roo_io::SD.setReadOnly(false);
    roo_io::SD.setMountingPolicy(roo_io::Filesystem::kMountReadWrite);
    roo_io::SD.setUnmountingPolicy(roo_io::Filesystem::kUnmountEagerly);
    SDFS.setMediaPresent(true);
    SDFS.setMountFailure(false);
    for (FakeSdSpiCard& card : wiring.cards) {
      card.setPresent(true);
      card.setOcr(0xc0ff8000);
      card.setResponseDelay(0);
    }
  }

  void TearDown() override {
    roo_io::SD.forceUnmount();
    SDFS.setMountFailure(false);
  }
};

// Verifies automatic detection on both controllers before any SD mount,
// including absent/inserted cards and deasserted CS after every probe.
TEST_F(PicoSdDetection, DetectsBeforeMountOnBothControllers) {
  for (int i = 0; i < 2; ++i) {
    roo_io::SD.setSPI(i == 0 ? SPI : SPI1);
    roo_io::SD.setCsPin(i == 0 ? 17 : 13);
    FakeSdSpiCard& card = wiring.cards[i];
    card.setPresent(false);
    EXPECT_EQ(roo_io::SD.checkMediaPresence(),
              roo_io::Filesystem::kMediaAbsent);
    card.setPresent(true);
    const uint32_t resets = card.resetCommands();
    EXPECT_EQ(roo_io::SD.checkMediaPresence(),
              roo_io::Filesystem::kMediaPresent);
    EXPECT_EQ(card.resetCommands(), resets + 1);
    EXPECT_FALSE(card.isSelected());
    EXPECT_FALSE(card.protocolError());
    EXPECT_FALSE(roo_io::SD.isMounted());
    EXPECT_FALSE(SDFS.mounted());
  }
}

// Verifies a mounted check uses CMD58 rather than resetting the card, leaves
// existing file I/O usable, detects removal and preserves borrowed SPI on
// unmount.
TEST_F(PicoSdDetection, MountedChecksPreserveCardAndMount) {
  FakeSdSpiCard& card = wiring.cards[0];
  roo_io::Mount mount = roo_io::SD.mount();
  ASSERT_TRUE(mount.ok());
  card.setReady(
      true);  // The FS fake does not emulate SD initialization traffic.
  const uint32_t resets = card.resetCommands();
  const uint32_t statuses = card.statusCommands();
  EXPECT_EQ(roo_io::SD.checkMediaPresence(), roo_io::Filesystem::kMediaPresent);
  EXPECT_EQ(card.resetCommands(), resets);
  EXPECT_EQ(card.statusCommands(), statuses + 1);
  EXPECT_TRUE(card.ready());
  EXPECT_FALSE(card.isSelected());
  auto output = mount.fopenForWrite("/status-check", roo_io::kTruncateIfExists);
  const roo_io::byte byte{0x42};
  EXPECT_EQ(output->writeFully(&byte, 1), 1u);
  output->close();
  EXPECT_EQ(mount.remove("/status-check"), roo_io::kOk);
  card.setPresent(false);
  EXPECT_EQ(roo_io::SD.checkMediaPresence(), roo_io::Filesystem::kMediaAbsent);
  EXPECT_TRUE(roo_io::SD.isMounted());
  EXPECT_EQ(card.resetCommands(), resets);
  mount.close();
  EXPECT_FALSE(roo_io::SD.isMounted());
  EXPECT_TRUE(FakePico().spiState(0).enabled);
  EXPECT_FALSE(card.protocolError());
}

// Verifies absence reports kNoMedia while a responding card with an unreadable
// filesystem retains kGenericMountError, without application callbacks.
TEST_F(PicoSdDetection, DistinguishesAbsenceFromMountFailure) {
  wiring.cards[0].setPresent(false);
  EXPECT_EQ(roo_io::SD.mount().status(), roo_io::kNoMedia);
  wiring.cards[0].setPresent(true);
  SDFS.setMountFailure(true);
  EXPECT_EQ(roo_io::SD.mount().status(), roo_io::kGenericMountError);
  SDFS.setMountFailure(false);
  EXPECT_TRUE(roo_io::SD.mount().ok());
}

// Verifies delayed/missing responses are bounded, invalid OCR values are
// rejected, and valid status responses still work after those failures.
TEST_F(PicoSdDetection, BoundsPollingAndRejectsInvalidOcr) {
  FakeSdSpiCard& card = wiring.cards[0];
  card.setResponseDelay(20);
  EXPECT_EQ(roo_io::SD.checkMediaPresence(), roo_io::Filesystem::kMediaAbsent);
  EXPECT_FALSE(card.isSelected());
  card.setResponseDelay(2);
  roo_io::Mount mount = roo_io::SD.mount();
  ASSERT_TRUE(mount.ok());
  card.setReady(true);
  card.setOcr(0);
  EXPECT_EQ(roo_io::SD.checkMediaPresence(), roo_io::Filesystem::kMediaAbsent);
  card.setOcr(0xffffffff);
  EXPECT_EQ(roo_io::SD.checkMediaPresence(), roo_io::Filesystem::kMediaAbsent);
  card.setOcr(0xc0ff8000);
  EXPECT_EQ(roo_io::SD.checkMediaPresence(), roo_io::Filesystem::kMediaPresent);
  EXPECT_TRUE(card.ready());
  EXPECT_FALSE(card.isSelected());
  EXPECT_FALSE(card.protocolError());
}
}  // namespace
