#include "roo_io_espressif_arduino/fs/arduino_sdmmc.h"

#ifdef SOC_SDMMC_HOST_SUPPORTED

#include "SD_MMC.h"

namespace roo_io {
namespace esp32 {

SdMmcFs::SdMmcFs(uint8_t clk, uint8_t cmd, uint8_t d0, const char* mountpoint,
                 int frequency, uint8_t max_open_files)
    : clk_(clk),
      cmd_(cmd),
      d0_(d0),
      d1_(-1),
      d2_(-1),
      d3_(-1),
      mode_1bit_(true),
      mountpoint_(mountpoint),
      frequency_(frequency),
      max_open_files_(max_open_files) {
  SD_MMC.setPins(clk, cmd, d0);
}

SdMmcFs::SdMmcFs(uint8_t clk, uint8_t cmd, uint8_t d0, uint8_t d1, uint8_t d2,
                 uint8_t d3, const char* mountpoint, int frequency,
                 uint8_t max_open_files)
    : clk_(clk),
      cmd_(cmd),
      d0_(d0),
      d1_(d1),
      d2_(d2),
      d3_(d3),
      mode_1bit_(false),
      mountpoint_(mountpoint),
      frequency_(frequency),
      max_open_files_(max_open_files) {
  SD_MMC.setPins(clk, cmd, d0, d1, d2, d3);
}

Filesystem::MediaPresence SdMmcFs::checkMediaPresence() {
  return SD_MMC.totalBytes() > 0 ? Filesystem::kMediaPresent
                                 : Filesystem::kMediaAbsent;
}

MountImpl::MountResult SdMmcFs::mountImpl(std::function<void()> unmount_fn) {
  bool result = SD_MMC.begin(mountpoint_.c_str(), mode_1bit_, false, frequency_,
                             max_open_files_);
  if (!result) {
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mountpoint_.c_str(), read_only_, unmount_fn)));
}

void SdMmcFs::unmountImpl() { SD_MMC.end(); }

}  // namespace esp32
}  // namespace roo_io

#endif