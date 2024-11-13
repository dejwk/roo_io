#include "roo_io/fs/esp32/sdspi.h"

#if defined(ROO_TESTING)

#include "roo_testing/devices/microcontroller/esp32/fake_esp32.h"

#endif

#if (defined(ESP32) || defined(ROO_TESTING))

#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_vfs_fat.h"
#include "roo_io/fs/posix/posix_mount.h"

namespace roo_io {
namespace esp32 {

SdSpiFs::SdSpiFs(uint8_t pin_sck, uint8_t pin_miso, uint8_t pin_mosi,
                 uint8_t pin_cs, spi_host_device_t spi_host)
    : spi_host_(spi_host),
      pin_sck_((gpio_num_t)pin_sck),
      pin_miso_((gpio_num_t)pin_miso),
      pin_mosi_((gpio_num_t)pin_mosi),
      pin_cs_((gpio_num_t)pin_cs),
      mount_point_("/sd"),
      max_files_(5),
      format_if_empty_(false),
      spi_frequency_(20000000) {}

void SdSpiFs::spi_config(uint8_t pin_sck, uint8_t pin_miso, uint8_t pin_mosi,
                         uint8_t pin_cs, spi_host_device_t spi_host) {
  pin_sck_ = (gpio_num_t)pin_sck;
  pin_miso_ = (gpio_num_t)pin_miso;
  pin_mosi_ = (gpio_num_t)pin_mosi;
  pin_cs_ = (gpio_num_t)pin_cs;
  spi_host_ = spi_host;
}

#if CONFIG_IDF_TARGET_ESP32

SdSpiFs::SdSpiFs() : SdSpiFs(18, 19, 23, -1, HSPI_HOST) {}

#elif CONFIG_IDF_TARGET_ESP32S2

SdSpiFs::SdSpiFs() : SdSpiFs(36, 37, 35, -1, HSPI_HOST) {}

#else

SdSpiFs::SdSpiFs() : SdSpiFs(36, 37, 35, -1, HSPI_HOST) {}

#endif

const char* SdSpiFs::mount_point() const { return mount_point_.c_str(); }

void SdSpiFs::set_mount_point(const char* mount_point) {
  mount_point_ = mount_point;
}

uint8_t SdSpiFs::max_files() const { return max_files_; }

void SdSpiFs::set_max_files(uint8_t max_files) { max_files_ = max_files; }

bool SdSpiFs::format_if_empty() const { return format_if_empty_; }

void SdSpiFs::set_format_if_empty(bool format_if_empty) {
  format_if_empty_ = format_if_empty;
}

MountImpl::MountResult SdSpiFs::mountImpl(std::function<void()> unmount_fn) {
  LOG(INFO) << "Mounting SD card";
#if defined(ROO_TESTING)
  mount_base_path_ = FakeEsp32().fs_root();
#else
  mount_base_path_.clear();
#endif
  mount_base_path_.append(mount_point_);

  // spi_bus_config_t spi_cfg = {.mosi_io_num = pin_mosi_,
  //                             .miso_io_num = pin_miso_,
  //                             .sclk_io_num = pin_sck_,
  //                             .quadwp_io_num = -1,
  //                             .quadhd_io_num = -1,
  //                             .data4_io_num = -1,
  //                             .data5_io_num = -1,
  //                             .data6_io_num = -1,
  //                             .data7_io_num = -1,
  //                             .max_transfer_sz = 0,
  //                             .flags = 0,
  //                             .intr_flags = 0};
  // esp_err_t ret;
  // ret = spi_bus_initialize(spi_host_, &spi_cfg, SDSPI_DEFAULT_DMA);
  // if (ret != ESP_OK) {
  //   LOG(ERROR) << "Failed to initialize bus.";
  //   return MountImpl::MountError(kMountError);
  // }
  sdmmc_host_t host = SDSPI_HOST_DEFAULT();

  sdspi_device_config_t dev_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  dev_config.host_id = spi_host_;
  dev_config.gpio_cs = pin_cs_;

  esp_vfs_fat_mount_config_t mount_config = {
      .format_if_mount_failed = format_if_empty_,
      .max_files = max_files_,
      .allocation_unit_size = 16 * 1024};

  esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_base_path_.c_str(), &host,
                                          &dev_config, &mount_config, &card_);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      LOG(ERROR) << "Failed to mount filesystem. "
                    "If you want the card to be formatted, set the "
                    "EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.";
    } else {
      LOG(ERROR) << "Failed to initialize the card (%s). "
                    "Make sure SD card lines have pull-up resistors in place."
                 << esp_err_to_name(ret);
    }
    return MountImpl::MountError(kMountError);
  }

  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mount_base_path_.c_str(), false, unmount_fn)));
}

void SdSpiFs::unmountImpl() {
  LOG(INFO) << "Unmounting SD card";
  CHECK_NOTNULL(card_);

  esp_err_t ret = esp_vfs_fat_sdcard_unmount(mount_base_path_.c_str(), card_);
  if (ret != ESP_OK) {
    LOG(ERROR) << "Unmounting card failed: " << esp_err_to_name(ret);
  }
  // spi_bus_free(spi_host_);
  card_ = nullptr;
  mount_base_path_.clear();
}

Filesystem::MediaPresence SdSpiFs::checkMediaPresence() {
  return kMediaPresenceUnknown;
}

SdSpiFs SDSPI;

}  // namespace esp32
}  // namespace roo_io

#endif
