#include "SPI.h"
#include "roo_io.h"
#include "roo_io/fs/arduino/sdfs.h"

static const int kPinSpiSck = 12;
static const int kPinSpiMiso = 13;
static const int kPinSpiMosi = 11;
static const int kPinSdCs = 10;

using namespace roo_io;

auto& sd = roo_io::SD;

void setup() {
  Serial.begin(115200);

  // Initialize the SPI bus.
  SPI.begin(kPinSpiSck, kPinSpiMiso, kPinSpiMosi);

  // Configure the SD filesystem.
  sd.setCsPin(kPinSdCs);
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
