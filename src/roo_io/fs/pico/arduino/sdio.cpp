#include "roo_io/fs/pico/arduino/sdio.h"

#if defined(ARDUINO) && defined(ARDUINO_ARCH_RP2040) && \
    (!defined(ROO_TESTING) || defined(ROO_IO_TEST_PICO_SDIO))

#include <utility>

#include "SDFS.h"
#include "roo_io/fs/arduino/mount.h"

namespace roo_io {

class PicoSdSdioFs::SdioFilesystem final : public sdfs::SDFSImpl {
 public:
  explicit SdioFilesystem(SdioConfig config) : config_(config) {}

  bool probe() {
    const bool present = getFs()->cardBegin(config_) || cardWasSelected();
    getFs()->end();
    return present;
  }

  bool responds() {
    // Finish any multi-block read before sending CMD13 on the active card.
    auto* card = getFs()->card();
    return card->syncDevice() && card->status() != CARD_STATUS_ERROR;
  }

  Status mountCard() {
    if (!getFs()->cardBegin(config_)) {
      const Status status = cardWasSelected() ? kGenericMountError : kNoMedia;
      getFs()->end();
      return status;
    }
    if (!getFs()->volumeBegin()) {
      getFs()->card()->syncDevice();
      getFs()->end();
      return kGenericMountError;
    }
    FsDateTime::setCallback(dateTimeCB);
    _mounted = true;
    return kOk;
  }

  void end() override {
    if (_mounted) getFs()->card()->syncDevice();
    sdfs::SDFSImpl::end();
    getFs()->end();
  }

 private:
  bool cardWasSelected() {
    // A width change or data transfer follows successful card selection.
    // Failures at that point still establish presence.
    switch (getFs()->sdErrorCode()) {
      case SD_CARD_ERROR_ACMD6:
      case SD_CARD_ERROR_ACMD13:
      case SD_CARD_ERROR_ACMD51:
      case SD_CARD_ERROR_READ_CRC:
      case SD_CARD_ERROR_READ_TIMEOUT:
        return true;
      default:
        return false;
    }
  }

  SdioConfig config_;
};

PicoSdSdioFs::PicoSdSdioFs(uint8_t clk, uint8_t cmd, uint8_t dat0,
                           float clock_divider)
    : impl_(std::make_shared<SdioFilesystem>(
          SdioConfig(clk, cmd, dat0, clock_divider))),
      fs_(impl_) {}

PicoSdSdioFs::~PicoSdSdioFs() {
  if (isMounted()) forceUnmount();
}

Filesystem::MediaPresence PicoSdSdioFs::checkMediaPresence() {
  const bool present = isMounted() ? impl_->responds() : impl_->probe();
  return present ? kMediaPresent : kMediaAbsent;
}

MountImpl::MountResult PicoSdSdioFs::mountImpl(
    std::function<void()> unmount_fn) {
  const Status status = impl_->mountCard();
  if (status != kOk) return MountImpl::MountError(status);
  return MountImpl::Mounted(
      std::make_unique<ArduinoMountImpl>(fs_, false, std::move(unmount_fn)));
}

void PicoSdSdioFs::unmountImpl() { fs_.end(); }

}  // namespace roo_io

#endif  // Arduino-Pico native SDIO
