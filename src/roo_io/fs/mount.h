#pragma once

#include <memory>

#include "roo_io/core/multipass_input_stream.h"
#include "roo_io/core/output_stream.h"
#include "roo_io/fs/directory.h"
#include "roo_io/fs/mount_impl.h"
#include "roo_io/fs/stat.h"

namespace roo_io {

/// Lightweight handle to a mounted filesystem instance.
///
/// Multiple `Mount` objects may share the same backend mount state.
class Mount {
 public:
  /// Creates a mount in the `kNotMounted` state.
  Mount() : Mount(kNotMounted) {}

  /// Non-copyable because mount handles own shared mount state.
  Mount(const Mount& other) = delete;
  /// Non-copyable because mount handles own shared mount state.
  Mount& operator=(const Mount& other) = delete;

  /// Move-constructs the mount and leaves `other` in the `kNotMounted` state.
  Mount(Mount&& other) {
    mount_ = other.mount_;
    status_ = other.status_;
    read_only_ = other.read_only_;
    other.close();
  }

  /// Move-assigns the mount and leaves `other` in the `kNotMounted` state.
  Mount& operator=(Mount&& other) {
    mount_ = other.mount_;
    status_ = other.status_;
    read_only_ = other.read_only_;
    other.close();
    return *this;
  }

  /// Returns the status of the mount itself.
  ///
  /// The result is one of:
  /// - `kOk`, if this handle still refers to an active backend mount.
  /// - `kNotMounted`, if the handle was default-constructed, closed, or
  ///   invalidated by filesystem unmount.
  /// - `kNoMedia`, `kOutOfMemory`, `kAccessDenied`, or
  ///   `kGenericMountError`, if mount creation failed for that reason.
  Status status() const {
    if (status_ == kOk && !mount_->active()) status_ = kNotMounted;
    return status_;
  }

  /// Returns whether the mount is healthy and can service filesystem operations.
  bool ok() const { return status() == kOk; }

  /// Returns whether `path` exists.
  ///
  /// Equivalent to `stat(path).exists()`.
  bool exists(const char* path) const { return stat(path).exists(); }

  /// Stats `path`, returning existence, type, size, or an error status.
  ///
  /// The returned `Stat` has one of these statuses:
  /// - `kOk`, if the operation completed successfully and the target exists.
  /// - `kInvalidPath`, if `path` is syntactically invalid.
  /// - `kNotFound`, if the target path, or any of its components, does not
  ///   exist.
  /// - `kNotDirectory`, or permissibly `kNotFound`, if an intermediate path
  ///   component exists but is not a directory.
  /// - `kAccessDenied`, `kOutOfMemory`, `kTooManyFilesOpen`, or
  ///   `kUnknownIOError`, if the backend reports that failure.
  /// - A copy of `status()` such as `kNotMounted` or `kNoMedia`, if the mount
  ///   is not healthy.
  Stat stat(const char* path) const {
    return status_ != kOk ? Stat(status_) : mount_->stat(path);
  }

  /// Removes the file at `path`.
  ///
  /// Returns one of:
  /// - `kOk`, if the file was successfully removed.
  /// - `kInvalidPath`, if `path` is syntactically invalid.
  /// - `kNotFound`, if the target path, or any of its components, does not
  ///   exist.
  /// - `kNotDirectory`, or permissibly `kNotFound`, if an intermediate path
  ///   component exists but is not a directory.
  /// - `kAccessDenied`, if permissions are insufficient.
  /// - `kReadOnlyFilesystem`, if the mount is read-only.
  /// - `kOutOfMemory`, `kNoSpaceLeftOnDevice`, or `kUnknownIOError`, if the
  ///   backend reports that failure.
  /// - A copy of `status()` such as `kNotMounted` or `kNoMedia`, if the mount
  ///   is not healthy.
  Status remove(const char* path) {
    return status_ != kOk ? status_
           : read_only_   ? kReadOnlyFilesystem
                          : mount_->remove(path);
  }

  /// Renames or moves an existing file or directory.
  ///
  /// Returns one of:
  /// - `kOk`, if the source was successfully moved.
  /// - `kInvalidPath`, if either path is syntactically invalid, or if
  ///   `pathTo` is a descendant of `pathFrom`.
  /// - `kNotFound`, if `pathFrom`, any of its components, or any intermediate
  ///   component of `pathTo` does not exist.
  /// - `kNotDirectory`, or permissibly `kNotFound`, if an intermediate path
  ///   component of either path exists but is not a directory.
  /// - `kFileExists` or `kDirectoryExists`, if the destination already exists.
  /// - `kAccessDenied`, if permissions are insufficient.
  /// - `kReadOnlyFilesystem`, if the mount is read-only.
  /// - `kOutOfMemory`, `kNoSpaceLeftOnDevice`, or `kUnknownIOError`, if the
  ///   backend reports that failure.
  /// - A copy of `status()` such as `kNotMounted` or `kNoMedia`, if the mount
  ///   is not healthy.
  Status rename(const char* pathFrom, const char* pathTo) {
    return status_ != kOk ? status_
           : read_only_   ? kReadOnlyFilesystem
                          : mount_->rename(pathFrom, pathTo);
  }

  /// Creates the directory at `path`.
  ///
  /// The parent directory must already exist.
  ///
  /// Returns one of:
  /// - `kOk`, if the directory was successfully created.
  /// - `kInvalidPath`, if `path` is syntactically invalid.
  /// - `kNotFound`, if an intermediate path component does not exist.
  /// - `kNotDirectory`, or permissibly `kNotFound`, if an intermediate path
  ///   component exists but is not a directory.
  /// - `kFileExists`, if the destination exists and is a file.
  /// - `kDirectoryExists`, if the destination exists and is a directory.
  /// - `kAccessDenied`, if permissions are insufficient.
  /// - `kReadOnlyFilesystem`, if the mount is read-only.
  /// - `kOutOfMemory`, `kNoSpaceLeftOnDevice`, or `kUnknownIOError`, if the
  ///   backend reports that failure.
  /// - A copy of `status()` such as `kNotMounted` or `kNoMedia`, if the mount
  ///   is not healthy.
  Status mkdir(const char* path) {
    return status_ != kOk ? status_
           : read_only_   ? kReadOnlyFilesystem
                          : mount_->mkdir(path);
  }

  /// Removes the empty directory at `path`.
  ///
  /// Returns one of:
  /// - `kOk`, if the directory was successfully removed.
  /// - `kInvalidPath`, if `path` is syntactically invalid.
  /// - `kNotFound`, if the target path, or any intermediate component, does
  ///   not exist.
  /// - `kNotDirectory`, if the target exists but is not a directory, or
  ///   permissibly if an intermediate component is not a directory.
  /// - `kDirectoryNotEmpty`, if the target directory exists but is not empty.
  /// - `kAccessDenied`, if permissions are insufficient.
  /// - `kReadOnlyFilesystem`, if the mount is read-only.
  /// - `kOutOfMemory` or `kUnknownIOError`, if the backend reports that
  ///   failure.
  /// - A copy of `status()` such as `kNotMounted` or `kNoMedia`, if the mount
  ///   is not healthy.
  Status rmdir(const char* path) {
    return status_ != kOk ? status_
           : read_only_   ? kReadOnlyFilesystem
                          : mount_->rmdir(path);
  }

  /// Opens the directory at `path` for iteration.
  ///
  /// The returned directory handle is in one of these states:
  /// - `kOk`, if the directory was successfully opened.
  /// - `kInvalidPath`, if `path` is syntactically invalid.
  /// - `kNotFound`, if the target path, or any intermediate component, does
  ///   not exist.
  /// - `kNotDirectory`, if the target exists but is not a directory, or
  ///   permissibly if an intermediate component is not a directory.
  /// - `kAccessDenied`, `kOutOfMemory`, `kTooManyFilesOpen`, or
  ///   `kUnknownIOError`, if the backend reports that failure.
  /// - A copy of `status()` such as `kNotMounted` or `kNoMedia`, if the mount
  ///   is not healthy.
  Directory opendir(const char* path) {
    return status_ != kOk ? Directory(status_)
                          : Directory(mount_->opendir(mount_, path));
  }

  /// Opens the file at `path` for seekable reading.
  ///
  /// The returned input stream is in one of these states:
  /// - `kOk`, if the file was successfully opened.
  /// - `kInvalidPath`, if `path` is syntactically invalid.
  /// - `kNotFound`, if the target path, or any intermediate component, does
  ///   not exist.
  /// - `kNotFile`, if the target exists but is not a file.
  /// - `kNotDirectory`, or permissibly `kNotFound`, if an intermediate path
  ///   component exists but is not a directory.
  /// - `kAccessDenied`, `kOutOfMemory`, `kTooManyFilesOpen`, or
  ///   `kUnknownIOError`, if the backend reports that failure.
  /// - A copy of `status()` such as `kNotMounted` or `kNoMedia`, if the mount
  ///   is not healthy.
  std::unique_ptr<MultipassInputStream> fopen(const char* path) {
    return status_ != kOk ? InputError(status_) : mount_->fopen(mount_, path);
  }

  /// Opens the file at `path` for writing using `update_policy`.
  ///
  /// The file is created if it does not already exist. If it does exist, the
  /// behavior is controlled by `update_policy`.
  ///
  /// The returned output stream is in one of these states:
  /// - `kOk`, if the file was successfully opened.
  /// - `kInvalidPath`, if `path` is syntactically invalid.
  /// - `kNotFound`, if the target path, or any intermediate component, does
  ///   not exist.
  /// - `kNotFile`, if the target exists but is not a file.
  /// - `kNotDirectory`, or permissibly `kNotFound`, if an intermediate path
  ///   component exists but is not a directory.
  /// - `kAccessDenied`, if permissions are insufficient.
  /// - `kReadOnlyFilesystem`, if the mount is read-only.
  /// - `kOutOfMemory`, `kTooManyFilesOpen`, or `kUnknownIOError`, if the
  ///   backend reports that failure.
  /// - A copy of `status()` such as `kNotMounted` or `kNoMedia`, if the mount
  ///   is not healthy.
  std::unique_ptr<OutputStream> fopenForWrite(const char* path,
                                              FileUpdatePolicy update_policy) {
    return status_ != kOk ? OutputError(status_)
           : read_only_   ? OutputError(kReadOnlyFilesystem)
                          : mount_->fopenForWrite(mount_, path, update_policy);
  }

  /// Returns whether the mount is known to be read-only.
  bool isReadOnly() const { return read_only_; }

  /// Closes this mount handle.
  ///
  /// This does not affect other mount handles for the same filesystem. Under
  /// `kUnmountEagerly`, closing the last healthy handle automatically unmounts
  /// the backend. Under `kUnmountLazily`, the backend may remain mounted for
  /// reuse.
  void close() {
    if (status_ == kOk) status_ = kNotMounted;
    mount_ = nullptr;
  }

 private:
  friend class Filesystem;

  Mount(Status error) : mount_(nullptr), status_(error), read_only_(false) {}

  Mount(std::shared_ptr<MountImpl> impl, bool read_only)
      : mount_(impl), status_(kOk), read_only_(read_only) {}

  std::shared_ptr<MountImpl> mount_;
  mutable Status status_;
  bool read_only_;
};

}  // namespace roo_io
