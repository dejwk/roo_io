#include "roo_io/fs/esp32/arduino/sdspi.h"

#if (defined ESP32 && defined ARDUINO)

#include "FS.h"
#include "SD.h"
#include "ff.h"
#include "sd_diskio.h"
#include "vfs_api.h"

namespace roo_io {

Filesystem::MediaPresence ArduinoSdSpiFs::checkMediaPresence() {
  FATFS* fsinfo;
  DWORD fre_clust;
  if (f_getfree("0:", &fre_clust, &fsinfo) != 0)
    return Filesystem::kMediaAbsent;
  uint64_t sectors = ((uint64_t)(fsinfo->csize)) * (fsinfo->n_fatent - 2);
  if (sectors > 0) return Filesystem::kMediaPresent;
  // Inconclusive?
  return Filesystem::kMediaPresenceUnknown;
}

MountImpl::MountResult ArduinoSdSpiFs::mountImpl(
    std::function<void()> unmount_fn) {
  pdrv_ = sdcard_init(cs_, &spi_, frequency_);
  if (pdrv_ == 0xFF) {
    return MountImpl::MountError(kGenericMountError);
  }
  if (!sdcard_mount(pdrv_, mountpoint_.c_str(), max_open_files_, false)) {
    sdcard_unmount(pdrv_);
    sdcard_uninit(pdrv_);
    pdrv_ = 0xFF;
    return MountImpl::MountError(kGenericMountError);
  }
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mountpoint_.c_str(), read_only_, unmount_fn)));
}

void ArduinoSdSpiFs::unmountImpl() {
  if (pdrv_ != 0xFF) {
    sdcard_unmount(pdrv_);

    sdcard_uninit(pdrv_);
    pdrv_ = 0xFF;
  }
}

}  // namespace roo_io

#endif
