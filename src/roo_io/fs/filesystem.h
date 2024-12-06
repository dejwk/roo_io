#pragma once

#include <memory>

#include "roo_io/fs/directory.h"
#include "roo_io/fs/mount.h"
#include "roo_io/fs/mount_impl.h"
#include "roo_io/status.h"

namespace roo_io {

// A high-level abstract class representing a mountable filesystem (e.g. an SD
// card slot).
class Filesystem {
 public:
  // Result of checkMediaPresence().
  enum MediaPresence {
    // Media (e.g. SD card) is known to be absent.
    kMediaAbsent = 0,

    // Media (e.g. SD card) is known to be present.
    kMediaPresent = 1,

    // Media presence cannot be determined.
    kMediaPresenceUnknown = 2
  };

  // Returns a new mount for the filesystem, or error status. The error can be:
  // * kNoMedia, when mount fails because there is no media (e.g. no SD card
  // inserted),
  // * kGenericMountError, if the cause of error is unknown.
  //
  // Multiple mount objects can coexist; they are all backed by the same mounted
  // filesystem. The filesystem gets mounted when the first mount object is
  // created, and it gets unmounted when the last mount object goes out of
  // scope. (If you want to eagerly mount a filesystem and keep it mounted,
  // simply create a mount object and keep it alive somewhere).
  Mount mount();

  // Returns true if the filesystem is currently mounted.
  bool isMounted() const { return !mount_.expired(); }

  // Checks whether media is present. Does not require the filesystem to be
  // mounted, and does not cause it to become mounted. Some implementations may not
  // be able to determine media presence; they can return kMediaPresenceUnknown
  // in such case.
  virtual MediaPresence checkMediaPresence() = 0;

 protected:
  Filesystem() = default;

  virtual MountImpl::MountResult mountImpl(
      std::function<void()> unmount_fn) = 0;

  virtual void unmountImpl() = 0;

 private:
  std::weak_ptr<MountImpl> mount_;
};

// Returns a pointer into path that points past the last '/' in the path.
const char* GetFileName(const char* path);

}  // namespace roo_io
