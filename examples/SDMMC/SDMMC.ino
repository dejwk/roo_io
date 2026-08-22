// This example shows how to use SDMMC filesystem on ESP32S3 using Arduino.

#ifdef ROO_TESTING
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#endif

#include "roo_io.h"
#include "roo_io/fs/esp32/arduino/sdmmc.h"

static const int kPinMmcClk = 12;
static const int kPinMmcCmd = 11;
static const int kPinMmcD0 = 13;

using namespace roo_io;

auto& sd = roo_io::SD_MMC;

void setup() {
  Serial.begin(115200);

#ifdef ROO_TESTING
  // The emulated SD card is backed by this local directory. Change the path to
  // expose a different directory; mounting at "/" makes it the card root.
  FakeEsp32().set_fs_root("examples");
  sd.setMountPoint("/");
#endif

  // Configure the SD filesystem.
  sd.setPins(kPinMmcClk, kPinMmcCmd, kPinMmcD0);
}

// Helper function to recursively write out the contents of the specified
// directory.
void printDir(Mount& mnt, Directory dir, int indent) {
  while (dir.read()) {
    for (int i = 0; i < indent; i++) Serial.print("  ");
    Serial.print(dir.entry().name());
    if (dir.entry().isDirectory()) {
      Serial.print("/\n");
      printDir(mnt, mnt.opendir(dir.entry().path()), indent + 1);
    } else {
      Serial.print("\n");
    }
  }
  if (dir.failed()) {
    LOG(ERROR) << "Failed to read directory: " << dir.status();
  }
  Serial.flush();
}

void loop() {
  {
    roo_io::Mount mnt = sd.mount();
    if (!mnt.ok()) {
      LOG(ERROR) << "Mount failed: " << mnt.status();
      delay(1000);
      return;
    }
    LOG(INFO) << "Mount successful.";
    LOG(INFO) << "SD card in use: " << sd.isInUse();
    // Recursively list the card content.
    printDir(mnt, mnt.opendir("/"), 0);
  }
  // The filesystem gets unmounted automatically when 'mnt' goes out of
  // scope.
  LOG(INFO) << "SD card in use: " << sd.isInUse();
  delay(5000);
}
