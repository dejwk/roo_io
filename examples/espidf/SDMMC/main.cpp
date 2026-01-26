// This example shows how to use SDMMC filesystem on ESP32S3 using esp-idf.

#include "roo_io.h"
#include "roo_io/fs/esp32/esp-idf/sdmmc.h"
#include "roo_threads.h"

static const int kPinMmcClk = 12;
static const int kPinMmcCmd = 11;
static const int kPinMmcD0 = 13;

using namespace roo_io;

auto& sd = roo_io::SDMMC;

// Helper function to recursively write out the contents of the specified
// directory.
void printDir(Mount& mnt, Directory dir, int indent) {
  while (dir.read()) {
    for (int i = 0; i < indent; i++) printf("  ");
    printf(dir.entry().name());
    if (dir.entry().isDirectory()) {
      printf("/\n");
      printDir(mnt, mnt.opendir(dir.entry().path()), indent + 1);
    } else {
      printf("\n");
    }
  }
  if (dir.failed()) {
    LOG(ERROR) << "Failed to read directory: " << dir.status();
  }
}

extern "C" void app_main() {
  // Configure the SD filesystem.
  sd.setPins(kPinMmcClk, kPinMmcCmd, kPinMmcD0);

  // Main loop.
  while (true) {
    {
      roo_io::Mount mnt = sd.mount();
      if (!mnt.ok()) {
        LOG(ERROR) << "Mount failed: " << mnt.status();
        roo::this_thread::sleep_for(roo_time::Millis(1000));
        continue;
      }
      LOG(INFO) << "Mount successful.";
      LOG(INFO) << "SD card in use: " << sd.isInUse();
      // Recursively list the card content.
      printDir(mnt, mnt.opendir("/"), 0);
    }
    // The filesystem gets unmounted automatically when 'mnt' goes out of
    // scope.
    LOG(INFO) << "SD card in use: " << sd.isInUse();
    roo::this_thread::sleep_for(roo_time::Millis(5000));
  }
}
