#pragma once

#if defined(ARDUINO) && defined(ARDUINO_ARCH_RP2040)

#include "roo_io/fs/arduino/mount.h"

namespace roo_io {

/// Adapts Arduino-Pico LittleFS deletion to preserve existing parent paths.
/// The core prunes empty ancestors; this mount recreates them after deletion.
/// Restoration is not atomic and does not preserve directory metadata.
class PicoLittleFsMountImpl : public ArduinoMountImpl {
 public:
  /// Borrows @p fs and releases the backend through @p unmount_fn on
  /// destruction.
  PicoLittleFsMountImpl(FS& fs, bool read_only,
                        std::function<void()> unmount_fn);

 protected:
  /// Deletes one entry and restores any ancestors pruned by the core.
  bool removeEntry(const char* path, bool directory) override;
};

}  // namespace roo_io

#endif  // ARDUINO && ARDUINO_ARCH_RP2040
