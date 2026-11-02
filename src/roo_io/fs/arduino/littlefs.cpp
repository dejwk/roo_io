#include "roo_io/fs/arduino/littlefs.h"
#if defined(ARDUINO_ARCH_RP2040)
#include "roo_io/fs/pico/arduino/littlefs_mount.h"
namespace roo_io {
MountImpl::MountResult ArduinoLittleFs::mountImpl(
    std::function<void()> unmount_fn) {
  if (!fs_.begin()) return MountImpl::MountError(kGenericMountError);
  return MountImpl::Mounted(
      std::unique_ptr<MountImpl>(new PicoLittleFsMountImpl(
          fs_, mountingPolicy() == kMountReadOnly, std::move(unmount_fn))));
}
void ArduinoLittleFs::unmountImpl() { fs_.end(); }
}  // namespace roo_io
#endif
