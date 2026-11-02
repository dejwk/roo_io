#include "roo_io/fs/pico/arduino/littlefs_mount.h"

#if defined(ARDUINO) && defined(ARDUINO_ARCH_RP2040)

#include <cstring>
#include <memory>
#include <utility>

namespace roo_io {

PicoLittleFsMountImpl::PicoLittleFsMountImpl(FS& fs, bool read_only,
                                             std::function<void()> unmount_fn)
    : ArduinoMountImpl(fs, read_only, std::move(unmount_fn)) {}

bool PicoLittleFsMountImpl::removeEntry(const char* path, bool directory) {
  size_t length = strlen(path);
  while (length > 1 && path[length - 1] == '/') --length;
  std::unique_ptr<char[]> parents(new char[length + 1]);
  memcpy(parents.get(), path, length);
  parents[length] = 0;
  char* slash = strrchr(parents.get(), '/');
  if (slash != nullptr) *slash = 0;
  if (!ArduinoMountImpl::removeEntry(path, directory)) return false;
  if (parents[0] == 0) return true;
  // Every prefix existed before deletion. Recreate only those the core pruned,
  // including when deletion is part of the shared rename replacement fallback.
  FS& fs = filesystem();
  for (char* end = parents.get() + 1;; ++end) {
    if (*end != '/' && *end != 0) continue;
    const char separator = *end;
    *end = 0;
    const bool present = fs.exists(parents.get()) || fs.mkdir(parents.get());
    *end = separator;
    if (!present) return false;
    if (separator == 0) return true;
  }
}

}  // namespace roo_io

#endif  // ARDUINO && ARDUINO_ARCH_RP2040
