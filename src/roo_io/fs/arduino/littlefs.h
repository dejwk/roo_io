#pragma once
#if defined(ARDUINO_ARCH_RP2040)
#include <LittleFS.h>

#include "roo_io/fs/filesystem.h"
namespace roo_io {
/// Mountable Arduino-Pico LittleFS wrapper. The borrowed FS object and this
/// wrapper must outlive all mounts and files. The application configures flash
/// layout and FSConfig (including auto-format) before mounting. Use one wrapper
/// per backend; calls and configuration changes require external serialization.
class ArduinoLittleFs : public Filesystem {
 public:
  /// Borrows @p fs (normally ::LittleFS) without mounting or formatting it.
  explicit ArduinoLittleFs(fs::FS& fs = ::LittleFS) : fs_(fs) {}
  /// Drops a cached lazy mount before the borrowed backend can be released.
  ~ArduinoLittleFs() override {
    if (isMounted()) forceUnmount();
  }

  /// Reports the soldered flash as present without mounting it.
  MediaPresence checkMediaPresence() override { return kMediaPresent; }

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;
  void unmountImpl() override;

 private:
  fs::FS& fs_;
};
}  // namespace roo_io
#endif
