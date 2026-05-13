#include "roo_io/fs/esp32/esp-idf/sdspi.h"

#if defined(ROO_TESTING)

#include "roo_testing/microcontrollers/esp32/fake_esp32.h"

#endif

#if (defined ESP_PLATFORM || defined ROO_TESTING)

#if !defined(MLOG_roo_io_fs)
#define MLOG_roo_io_fs 0
#endif

#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_vfs_fat.h"
#include "roo_io/fs/posix/posix_mount.h"
#include "roo_logging.h"
#if !defined(ROO_TESTING)
#include "sd_protocol_defs.h"
#include "roo_io/fs/esp32/esp-idf/internal/sd_spi_probe.h"
#endif

namespace roo_io {

SdSpiFs::SdSpiFs(uint8_t cs_pin, spi_host_device_t spi_host, uint32_t frequency)
    : BaseEsp32VfsFilesystem(frequency, "/sd"),
      cs_pin_((gpio_num_t)cs_pin),
      spi_host_(spi_host) {}

MountImpl::MountResult SdSpiFs::mountImpl(std::function<void()> unmount_fn) {
  MLOG(roo_io_fs) << "Mounting SD card";
  if (checkMediaPresence() == kMediaAbsent) {
    return MountImpl::MountError(kNoMedia);
  }
#if defined(ROO_TESTING)
  mount_base_path_ = FakeEsp32().fs_root();
#else
  mount_base_path_.clear();
#endif
  mount_base_path_.append(mountPoint());

  esp_err_t ret;

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();

  sdspi_device_config_t dev_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  dev_config.host_id = spi_host_;
  dev_config.gpio_cs = cs_pin_;

  esp_vfs_fat_mount_config_t mount_config = {};
  mount_config.format_if_mount_failed = formatIfMountFailed(),
  mount_config.max_files = maxOpenFiles(),
  mount_config.allocation_unit_size = 16 * 1024;

  if (checkMediaPresence() == kMediaAbsent) {
    return MountImpl::MountError(kNoMedia);
  }
  ret = esp_vfs_fat_sdspi_mount(mount_base_path_.c_str(), &host, &dev_config,
                                &mount_config, &card_);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      LOG(ERROR) << "Failed to mount filesystem. If you want the card to be "
                    "formatted, use setFormatIfEmpty(true)";
      return MountImpl::MountError(kGenericMountError);
    } else if (ret == ESP_ERR_TIMEOUT) {
      return MountImpl::MountError(kNoMedia);
    } else {
      LOG(ERROR) << "Failed to initialize the card (" << esp_err_to_name(ret)
                 << ").";
    }
    return MountImpl::MountError(kGenericMountError);
  }

  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mount_base_path_.c_str(), readOnly(), unmount_fn)));
}

void SdSpiFs::unmountImpl() {
  MLOG(roo_io_fs) << "Unmounting SD card";
  CHECK_NOTNULL(card_);

  esp_err_t ret = esp_vfs_fat_sdcard_unmount(mount_base_path_.c_str(), card_);
  if (ret != ESP_OK) {
    LOG(ERROR) << "Unmounting card failed: " << esp_err_to_name(ret);
  }
  card_ = nullptr;
  mount_base_path_.clear();
}

Filesystem::MediaPresence SdSpiFs::checkMediaPresence() {
#if defined(ROO_TESTING)
  return kMediaPresenceUnknown;
#else
  if (card_ != nullptr) {
    // Already mounted — send CMD58 (READ_OCR) through the mounted SDSPI
    // driver's existing device handle.  No retries, no extra SPI device.
    sdmmc_command_t cmd = {};
    cmd.opcode = SD_READ_OCR;
    cmd.arg = 0;
    cmd.flags = SCF_CMD_AC | SCF_RSP_R3;
    esp_err_t err = card_->host.do_transaction(card_->host.slot, &cmd);

    if (err != ESP_OK) return kMediaAbsent;
    // A real card's OCR has MMC_OCR_MEM_READY (bit 31) plus voltage window
    // bits set.  Absent card reads as either:
    //  - 0x00000000 (MISO driven low by another bus device)
    //  - 0xFFFFFFFF (MISO pulled high)
    uint32_t ocr = cmd.response[0];
    return (ocr == 0 || ocr == 0xFFFFFFFF) ? kMediaAbsent : kMediaPresent;
  }
  // Not mounted — fast direct CMD0 probe.  Much faster than
  // sdspi_host_init_device + sdmmc_card_init which does multiple retries.
  return internal::SdSpiProbeCardEspIdf(spi_host_, cs_pin_) ? kMediaPresent
                                                            : kMediaAbsent;
#endif
}

SdSpiFs CreateSdSpiFs() { return SdSpiFs(); }

SdSpiFs SDSPI = CreateSdSpiFs();

}  // namespace roo_io

#endif  // (defined ESP_PLATFORM || defined ROO_TESTING)
