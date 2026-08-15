#pragma once

#if defined(ROO_TESTING)

#include <sys/stat.h>

#include <string>

#include "roo_testing/microcontrollers/esp32/fake_esp32.h"

namespace roo_io {
namespace internal {

inline std::string HostTestMountPath(const char* mount_point) {
  std::string path = FakeEsp32().fs_root();
  path.append(mount_point);
  return path;
}

inline bool HostTestMountPointExists(const char* mount_point) {
  struct stat status;
  std::string path = HostTestMountPath(mount_point);
  return stat(path.c_str(), &status) == 0 && S_ISDIR(status.st_mode);
}

}  // namespace internal
}  // namespace roo_io

#endif  // ROO_TESTING
