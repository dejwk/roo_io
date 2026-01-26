#include "driver/spi_master.h"
#include "roo_io.h"
#include "roo_io/fs/esp32/esp-idf/sdspi.h"
#include "roo_threads.h"

static const int kPinSpiSck = 12;
static const int kPinSpiMiso = 13;
static const int kPinSpiMosi = 11;
static const int kPinSdCs = 10;

using namespace roo_io;

auto& sd = roo_io::SDSPI;

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
  // Initialize the SPI bus.
  spi_bus_config_t buscfg = {.mosi_io_num = kPinSpiMosi,
                             .miso_io_num = kPinSpiMiso,
                             .sclk_io_num = kPinSpiSck,
                             .quadwp_io_num = -1,
                             .quadhd_io_num = -1,
                             .max_transfer_sz = 0};
  esp_err_t ret;
  ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
  CHECK_EQ(ret, ESP_OK) << "Failed to initialize SPI bus.";

  // Configure the SD filesystem.
  sd.setCsPin(kPinSdCs);

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
