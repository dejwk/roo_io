#include "roo_io_sdfat/fs/sdfat_spi.h"

#include "roo_io_sdfat/fs/sdfat_mount.h"

#ifdef ROO_TESTING
#include <string>

#include "roo_io/fs/posix/posix_mount.h"
#include "roo_testing/devices/microcontroller/esp32/fake_esp32.h"
#endif

namespace roo_io {

SdFatSpiFs::SdFatSpiFs(uint8_t sck_pin, uint8_t miso_pin, uint8_t mosi_pin,
                       uint8_t ss_pin, SdFat& sd, decltype(SPI)& spi,
                       uint32_t freq)
    : sd_(sd),
      spi_(spi),
      sck_pin_(sck_pin),
      miso_pin_(miso_pin),
      mosi_pin_(mosi_pin),
      ss_pin_(ss_pin),
      frequency_(freq) {}

Filesystem::MediaPresence SdFatSpiFs::checkMediaPresence() {
#ifdef ROO_TESTING
  return kMediaPresent;
#else
  if (isMounted()) {
    cid_t cid;
    return sd_.card()->readCID(&cid) ? kMediaPresent : kMediaAbsent;
  } else {
    bool ok =
        sd_.cardBegin(SdSpiConfig(ss_pin_, SHARED_SPI, frequency_, &spi_));
    if (ok) {
      sd_.end();
    }
    return ok ? kMediaPresent : kMediaAbsent;
  }
#endif
}

MountImpl::MountResult SdFatSpiFs::mountImpl(std::function<void()> unmount_fn) {
  if (!sd_.begin(SdSpiConfig(ss_pin_, SHARED_SPI, frequency_, &spi_))) {
    return MountImpl::MountError(kGenericMountError);
  }
#ifdef ROO_TESTING
  std::string mount_point = FakeEsp32().fs_root();
  mount_point += "/sd";
  return MountImpl::Mounted(std::unique_ptr<MountImpl>(
      new PosixMountImpl(mount_point.c_str(), false, unmount_fn)));
#else
  return MountImpl::Mounted(
      std::unique_ptr<MountImpl>(new SdFatMountImpl(sd_, false, unmount_fn)));
#endif
}

void SdFatSpiFs::unmountImpl() {
  sd_.end();
  // SdFat calls spi.close() on sd_.end(), deiniting the entire bus. To work
  // around that, we reinit the bus right away.
  spi_.begin(sck_pin_, miso_pin_, mosi_pin_);
}

// SdFatFs SDFAT;

}  // namespace roo_io