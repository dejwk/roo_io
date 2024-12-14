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
  virtual ~Filesystem() = default;

  // Result of checkMediaPresence().
  enum MediaPresence {
    // Media (e.g. SD card) is known to be absent.
    kMediaAbsent = 0,

    // Media (e.g. SD card) is known to be present.
    kMediaPresent = 1,

    // Media presence cannot be determined.
    kMediaPresenceUnknown = 2
  };

  // Decides the level of access allowed for new mounts. Does not affect
  // existing mounts.
  enum MountingPolicy {
    // New mounts can both read and write (as long as the filesystem supports
    // that).
    kMountReadWrite,

    // New mounts cannot write to the file system.
    kMountReadOnly,

    // New mounts are disallowed.
    kMountDisabled,
  };

  // Decides how aggressive the filesystem gets unmounted when no longer used.
  enum UnmountingPolicy {
    // Unmount as soon as all mount objects go out of scope.
    kLazyUnmount = 0,

    // Keep mounted until explicit call to unmount.
    kEagerUnmount = 1
  };

  // Returns a new mount for the filesystem, or error status. The error can be:
  // * kNoMedia, when mount fails because there is no media (e.g. no SD card
  // inserted),
  // * kGenericMountError, if the cause of error is unknown.
  //
  // Multiple mount objects can be independently created. They are all backed by
  // the same mounted filesystem. The filesystem gets mounted when the first
  // mount object is created.
  //
  // Generally you don't need to, and shouldn't, explicitly unmount the
  // filesystem. If `unmountingPolicy()' is 'kEagerUnmount', the filesystem gets
  // automatically unmounted when the last mount object gets out of scope.
  // If that policy is 'kLazyUnmount', the filesystem is kept mounted, to be
  // reused by future created mount objects.
  Mount mount();

  // Returns true if the filesystem is currently mounted.
  bool isMounted() const { return !mount_.expired(); }

  // Returns true if there are any user-created live mounts referencing the
  // filesystem. If unmounting policy is 'kEagerUnmount', it is always the same
  // as 'isMounted()'. With 'kLazyUnmount', 'isUnUse()' can be false while
  // 'isMounted()' is still true.
  bool isInUse() const {
    return mount_.use_count() > (unmounting_policy_ == kEagerUnmount ? 0 : 1);
  }

  // Checks whether media is present. Does not require the filesystem to be
  // mounted, and does not cause it to become mounted. Some implementations may
  // not be able to determine media presence; they can return
  // kMediaPresenceUnknown in such case.
  virtual MediaPresence checkMediaPresence() = 0;

  // Returns the current value of the mounting policy.
  MountingPolicy mountingPolicy() const { return mounting_policy_; }

  // Sets the mounting policy, which decides what do to when `mount()` gets
  // called.
  void setMountingPolicy(MountingPolicy mounting_policy) {
    mounting_policy_ = mounting_policy;
  }

  // Returns the current value of the unmounting policy.
  UnmountingPolicy unmountingPolicy() const { return unmounting_policy_; }

  // Sets unmounting policy, which decides how aggressively to unmount the
  // filesystem when it stops being used.
  //
  // If switching from lazy to eager, and no mount objects currently exist, the
  // filesystem gets unmounted. Otherwise, the call has no immediate effect.
  void setUnmountingPolicy(UnmountingPolicy unmounting_policy);

  // Invalidates all existing mount objects, and unmounts the filesystem
  // immediately. The mounts will return 'kNotMounted' from any subsequent
  // calls.
  //
  // New mounts can still be created, as long as the mounting policy allows it.
  //
  // Prefer relying on automount. This method is indended for use in special
  // circumstances, such as forceful shutdown.
  void forceUnmount();

 protected:
  Filesystem()
      : mounting_policy_(kMountReadWrite), unmounting_policy_(kEagerUnmount) {}

  virtual MountImpl::MountResult mountImpl(
      std::function<void()> unmount_fn) = 0;

  virtual void unmountImpl() = 0;

 private:
  std::weak_ptr<MountImpl> mount_;

  // Used when unmounting_policy_ == kLazyUnmount, to keep the mount alive.
  std::shared_ptr<MountImpl> lazy_unmount_;

  MountingPolicy mounting_policy_;
  UnmountingPolicy unmounting_policy_;
};

// Returns a pointer into path that points past the last '/' in the path.
const char* GetFileName(const char* path);

}  // namespace roo_io
