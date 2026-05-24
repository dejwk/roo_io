#pragma once

#include <memory>

#include "roo_io/fs/directory.h"
#include "roo_io/fs/mount.h"
#include "roo_io/fs/mount_impl.h"
#include "roo_io/status.h"

namespace roo_io {

/// High-level abstraction for a mountable filesystem, such as an SD card slot.
///
/// `Mount` handles created from the same filesystem share one backend mount
/// when possible. In normal use, callers should rely on the filesystem's
/// automatic unmount behavior instead of explicitly calling `forceUnmount()`.
class Filesystem {
 public:
  /// Destroys the filesystem abstraction.
  virtual ~Filesystem() = default;

  /// Result returned by `checkMediaPresence()`.
  enum MediaPresence {
    /// Media is known to be absent.
    kMediaAbsent = 0,

    /// Media is known to be present.
    kMediaPresent = 1,

    /// Media presence cannot be determined.
    kMediaPresenceUnknown = 2
  };

  /// Controls the access mode allowed for newly created mounts.
  enum MountingPolicy {
    /// Future mounts may read and write when the backend supports both.
    kMountReadWrite,

    /// Future mounts are forced read-only even if the backend supports writes.
    kMountReadOnly,

    /// Future `mount()` calls return `kNotMounted` without touching the backend.
    kMountDisabled,
  };

  /// Controls whether the backend mount is dropped automatically when no
  /// healthy `Mount` handles remain.
  enum UnmountingPolicy {
    /// Keep the backend mount cached after the last `Mount` handle is dropped.
    kUnmountLazily = 0,

    /// Drop the backend mount as soon as the last `Mount` handle is dropped.
    kUnmountEagerly = 1
  };

  /// Returns a new mount handle for the filesystem or an error state.
  ///
  /// Possible mount-handle statuses are:
  /// - `kOk`, when mounting succeeded.
  /// - `kNotMounted`, when mounting is disabled.
  /// - `kNoMedia`, when mounting fails because backing media is absent.
  /// - `kGenericMountError`, when mounting fails for an unknown reason.
  ///
  /// Multiple mount handles may coexist and share the same mounted backend.
  /// The backend is mounted when the first handle is created. Generally you do
  /// not need to, and should not, explicitly unmount the filesystem:
  /// `kUnmountEagerly` automatically drops the backend mount when the last
  /// healthy `Mount` handle is destroyed or closed, while `kUnmountLazily`
  /// keeps it mounted for reuse by future handles.
  Mount mount();

  /// Returns whether the filesystem backend is currently mounted.
  bool isMounted() const { return !mount_.expired(); }

  /// Returns whether any user-visible mount handles still reference this filesystem.
  ///
  /// Under `kUnmountLazily`, this can become `false` while `isMounted()`
  /// remains `true` because the filesystem keeps the backend mount cached for
  /// reuse.
  bool isInUse() const {
    return mount_.use_count() > (unmounting_policy_ == kUnmountEagerly ? 0 : 1);
  }

  /// Checks whether backing media is present without forcing a mount.
  ///
  /// Implementations should not mount the filesystem as a side effect.
  virtual MediaPresence checkMediaPresence() = 0;

  /// Returns the current mounting policy.
  MountingPolicy mountingPolicy() const { return mounting_policy_; }

  /// Sets the mounting policy used by future `mount()` calls.
  ///
  /// Existing mount handles keep their current access mode.
  void setMountingPolicy(MountingPolicy mounting_policy) {
    mounting_policy_ = mounting_policy;
  }

  /// Returns the current unmounting policy.
  UnmountingPolicy unmountingPolicy() const { return unmounting_policy_; }

  /// Sets how the backend mount is retained after the last live `Mount` closes.
  ///
  /// If switching from lazy to eager, any cached lazy mount is released
  /// immediately; if no other `Mount` handles still exist, this also causes an
  /// automatic unmount.
  void setUnmountingPolicy(UnmountingPolicy unmounting_policy);

  /// Invalidates all existing mount handles and unmounts immediately.
  ///
  /// Handles that were previously `kOk` start reporting `kNotMounted`
  /// afterward. New mounts can still be created later if mounting remains
  /// enabled. Prefer relying on the automatic unmount behavior from
  /// `unmountingPolicy()`; `forceUnmount()` is intended for exceptional cases
  /// such as shutdown.
  void forceUnmount();

 protected:
  Filesystem()
      : mounting_policy_(kMountReadWrite),
        unmounting_policy_(kUnmountEagerly) {}

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

/// Returns a pointer inside `path` positioned past the last `/` separator.
const char* GetFileName(const char* path);

}  // namespace roo_io
