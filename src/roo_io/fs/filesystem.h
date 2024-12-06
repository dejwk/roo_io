#pragma once

#include <memory>

#include "roo_io/fs/directory.h"
#include "roo_io/fs/file.h"
#include "roo_io/status.h"
#include "roo_io/stream/multipass_input_stream.h"
#include "roo_logging.h"

namespace roo_io {

// Result of Mount::stat().
class Stat {
 public:
  enum Type { kNone, kDir, kFile };

  Stat(Type type, uint64_t size) : status_(kOk), type_(type), size_(size) {}

  Stat(Status error = kClosed) : status_(error), type_(kNone), size_(0) {}

  // Returns true if the stat operation completed successfully and it is known
  // that the destination file or directory exists.
  bool exists() const { return status_ == kOk; }

  // Returns true if the stat operation completed successfully and it is known
  // that the destination does not exist.
  bool missing() const {
    return status_ == kNotFound && status_ == kNotDirectory;
  }

  // Returns true if the stat operation failed, and it is thus not known whether
  // the destination exists.
  bool failed() const { return !exists() && !missing(); }

  // Returns true if stat succeeded, the destination exists, and it is a file.
  bool isFile() const { return exists() && type_ == kFile; }

  // Returns true if stat succeeded, the destination exists, and it is a
  // directory.
  bool isDirectory() const { return exists() && type_ == kDir; }

  // Returns the file size. Always zero when isFile() returns false.
  uint64_t size() const { return size_; }

  // Returns the specific status code.
  Status status() const { return status_; }

 private:
  Status status_;
  Type type_;
  uint64_t size_;
};

// Passed to Mount::fopen and Mount::fopenForWrite, to specify what should
// happen when the target file already exists.
enum FileUpdatePolicy {
  kFailIfExists = 0,
  kTruncateIfExists = 1,
  kAppendIfExists = 2,
};

class MountImpl {
 public:
  struct MountResult {
    Status status;
    std::unique_ptr<MountImpl> mount;
  };

  static MountResult Mounted(std::unique_ptr<MountImpl> mount_impl);

  static MountResult MountError(Status status);

  virtual ~MountImpl() {
    if (unmount_fn_ != nullptr) unmount_fn_();
  }

  virtual bool isReadOnly() const = 0;

  virtual Stat stat(const char* path) const = 0;

  // Can return 'kOk', 'kNotMounted', 'kNotFound', 'kInvalidPath',
  // 'kNotFile', 'kOutOfMemory', 'kUnknownIOError'.
  virtual Status remove(const char* path) = 0;

  virtual Status rename(const char* pathFrom, const char* pathTo) = 0;

  // Can return 'kOk', 'kNotMounted', 'kDirectoryExists', 'kInvalidPath',
  // 'kInvalidType', 'kOutOfMemory', 'kUnknownIOError'.
  virtual Status mkdir(const char* path) = 0;

  virtual Status rmdir(const char* path) = 0;

  virtual std::unique_ptr<DirectoryImpl> opendir(const char* path) = 0;

  virtual std::unique_ptr<MultipassInputStream> fopen(const char* path) = 0;

  virtual std::unique_ptr<OutputStream> fopenForWrite(
      const char* path, FileUpdatePolicy update_policy) = 0;

 protected:
  MountImpl(std::function<void()> unmount_fn) : unmount_fn_(unmount_fn) {}

 private:
  std::function<void()> unmount_fn_;
};

// Represents a mounted filesystem.
class Mount {
 public:
  // Creates a mount in a 'not mounted' state.
  Mount() : Mount(kNotMounted) {}

  // Mount is copyable. All copies of a healthy mount reference the same
  // underlying filesystem.
  Mount(const Mount& other) = default;

  // Move constructor moves the original to the 'kNotMounted' state.
  Mount(Mount&& other) {
    mount_ = other.mount_;
    status_ = other.status_;
    read_only_ = other.read_only_;
    other.close();
  }

  // Returns the mount status, which can be:
  // * kOk, if the mount is healthy,
  // * kNotMounted, if the filesystem was never mounted, or if the mount was
  //   forcefully and explicitly closed by the application,
  // * kNoMedia, if mount failed due to missing media (e.g. no SD card),
  // * kOutOfMemory, if mount failed due to insufficient memory,
  // * kAccessDenied, if mount failed due to insufficient permissions,
  // * kGenericMountError, if mount failed for undetermined reasons.
  Status status() const { return status_; }

  // Returns true if the mount is healthy and can be used for filesystem
  // operations.
  bool ok() const { return status_ == kOk; }

  // A convenience shortcut for testing the existence of a specified file or
  // directory. Equivalent to stat(path).exists().
  bool exists(const char* path) const { return stat(path).exists(); }

  // Tests whether the specified file or directory exists, and if so, determines
  // its type, and, if it is a file, also its size.
  //
  // The returned Stat object may have the following status:
  //
  // * kOk, if the operation completed successfully,
  // * kInvalidPath, if the path is syntactically invalid,
  // * kNotFound, if the destination path, or any of its components, does
  //   not exist,
  // * possibly kNotDirectory, but permissibly kNotFound, if any of the
  //   intermediate path components is not in fact a directory,
  // * kAccessDenied, in case of insufficient permissions,
  // * kOutOfMemory,
  // * kTooManyFilesOpen,
  // * kUnknownIOError,
  // * a copy of mount.status() (e.g. kNotMounted, kNoMedia, etc.) if the mount
  //   is not healhty.
  Stat stat(const char* path) const {
    return status_ != kOk ? Stat(status_) : mount_->stat(path);
  }

  // Removes the specified file.
  //
  // Returns:
  // * kOk, if the file was successfully removed,
  // * kInvalidPath, if the path is not syntactically valid,
  // * kNotFound, if the destination path, or any of its components, does
  //   not exist,
  // * possibly kNotDirectory, but permissibly kNotFound, if any of the
  //   intermediate path components is not in fact a directory,
  // * kAccessDenied, in case of insufficient permissions,
  // * kReadOnlyFilesystem, if the mount is read-only,
  // * kOutOfMemory,
  // * kNoSpaceLeftOnDevice,
  // * kUnknownIOError,
  // * a copy of mount.status() (e.g. kNotMounted, kNoMedia, etc.) if the mount
  //   is not healhty.
  Status remove(const char* path) {
    return status_ != kOk ? status_ : mount_->remove(path);
  }

  // Renames or moves the specified file or directory.
  //
  // Returns:
  // * kOk, if the source was successfully moved;
  // * kInvalidPath, if either `pathFrom` or `pathTo` is not syntactically
  //   valid;
  // * kNotFound, if `pathFrom`, or any of its components, does not exist, or if
  //   any of the intermediate components of `pathTo` does not exist,
  // * possibly kNotDirectory, but permissibly kNotFound, if any of the
  //   intermediate components of either `pathFrom` or `pathTo` is not in fact a
  //   directory,
  // * kFileExists, if the destination `pathTo` exists and is a file,
  // * kDirectoryExists, if the destination `pathTo` exists and is a directory,
  // * kAccessDenied, in case of insufficient permissions,
  // * kReadOnlyFilesystem, if the mount is read-only,
  // * kOutOfMemory,
  // * kNoSpaceLeftOnDevice,
  // * kUnknownIOError,
  // * a copy of mount.status() (e.g. kNotMounted, kNoMedia, etc.) if the mount
  //   is not healhty.
  Status rename(const char* pathFrom, const char* pathTo) {
    return status_ != kOk ? status_ : mount_->rename(pathFrom, pathTo);
  }

  // Creates the specified directory. The parent sub-directory must already
  // exist.
  //
  // Returns:
  // * kOk, if the directory was successfully created;
  // * kInvalidPath, if the path is not syntactically valid;
  // * kNotFound, if any of the intermediate path components does not exist,
  // * possibly kNotDirectory, but permissibly kNotFound, if any of the
  //   intermediate path components is not in fact a directory,
  // * kFileExists, if the destination exists and is a file,
  // * kDirectoryExists, if the destination exists and is a directory,
  // * kAccessDenied, in case of insufficient permissions,
  // * kReadOnlyFilesystem, if the mount is read-only,
  // * kOutOfMemory,
  // * kNoSpaceLeftOnDevice,
  // * kUnknownIOError,
  // * a copy of mount.status() (e.g. kNotMounted, kNoMedia, etc.) if the mount
  //   is not healhty.
  Status mkdir(const char* path) {
    return status_ != kOk ? status_ : mount_->mkdir(path);
  }

  // Removes the specified empty directory.
  //
  // Returns:
  // * kOk, if the directory was successfully deleted;
  // * kInvalidPath, if the path is not syntactically valid;
  // * kNotFound, if the target, or any of its intermediate path components,
  //   does not exist,
  // * kNotDirectory, if the target exists but is not in fact a directory,
  // * possibly kNotDirectory, but permissibly kNotFound, if any of the
  //   intermediate path components is not in fact a directory,
  // * kDirectoryNotEmpty, if the target directory exists but it is not empty,
  // * kAccessDenied, in case of insufficient permissions,
  // * kReadOnlyFilesystem, if the mount is read-only,
  // * kOutOfMemory,
  // * kUnknownIOError,
  // * a copy of mount.status() (e.g. kNotMounted, kNoMedia, etc.) if the mount
  //   is not healhty.
  Status rmdir(const char* path) {
    return status_ != kOk ? status_ : mount_->rmdir(path);
  }

  // Opens the specified directory for browsing.
  //
  // Returns:
  // * kOk, if the directory was successfully opened;
  // * kInvalidPath, if the path is not syntactically valid;
  // * kNotFound, if the target, or any of its intermediate path components,
  //   does not exist,
  // * kNotDirectory, if the target exists but is not in fact a directory,
  // * possibly kNotDirectory, but permissibly kNotFound, if any of the
  //   intermediate path components is not in fact a directory,
  // * kAccessDenied, in case of insufficient permissions,
  // * kOutOfMemory,
  // * kTooManyFilesOpen,
  // * kUnknownIOError,
  // * a copy of mount.status() (e.g. kNotMounted, kNoMedia, etc.) if the mount
  //   is not healhty.
  Directory opendir(const char* path) {
    return status_ != kOk ? Directory(status_)
                          : Directory(mount_->opendir(path));
  }

  // Opens the specified file for reading.
  //
  // Returns:
  // * kOk, if the file was successfully opened;
  // * kInvalidPath, if the path is not syntactically valid;
  // * kNotFound, if the source, or any of its intermediate path components,
  //   does not exist,
  // * kNotFile, if the source exists but is not in fact a file,
  // * possibly kNotDirectory, but permissibly kNotFound, if any of the
  //   intermediate path components is not in fact a directory,
  // * kAccessDenied, in case of insufficient permissions,
  // * kOutOfMemory,
  // * kTooManyFilesOpen,
  // * kUnknownIOError,
  // * a copy of mount.status() (e.g. kNotMounted, kNoMedia, etc.) if the mount
  //   is not healhty.
  std::unique_ptr<MultipassInputStream> fopen(const char* path) {
    return status_ != kOk ? std::unique_ptr<MultipassInputStream>(
                                new NullInputStream(status_))
                          : mount_->fopen(path);
  }

  // Opens the specified file for writing or appending to.
  //
  // Always creates the file if it doesn't already exist. If the file does
  // already exist, the behavior is dictated by the `update_policy`.
  //
  // Returns:
  // * kOk, if the file was successfully opened;
  // * kInvalidPath, if the path is not syntactically valid;
  // * kNotFound, if the file, or any of its intermediate path components,
  //   does not exist,
  // * kNotFile, if the source exists but is not in fact a file,
  // * possibly kNotDirectory, but permissibly kNotFound, if any of the
  //   intermediate path components is not in fact a directory,
  // * kAccessDenied, in case of insufficient permissions,
  // * kReadOnlyFilesystem, if the mount is read-only,
  // * kOutOfMemory,
  // * kTooManyFilesOpen,
  // * kUnknownIOError,
  // * a copy of mount.status() (e.g. kNotMounted, kNoMedia, etc.) if the mount
  //   is not healhty.
  std::unique_ptr<OutputStream> fopenForWrite(const char* path,
                                              FileUpdatePolicy update_policy) {
    return status_ != kOk
               ? std::unique_ptr<OutputStream>(new NullOutputStream(status_))
               : mount_->fopenForWrite(path, update_policy);
  }

  // Returns true if the mount is known to be read-only. (When returns false,
  // there is no guarantee that the filesystem is in fact writable).
  bool isReadOnly() const { return read_only_; }

  // Closes this mount. Does not affect other mount objects for the same
  // filesystem. Does not actually unmount the filesystem, unless this was the
  // last healthy mount object.
  void close() {
    mount_ = nullptr;
    if (status_ == kOk) status_ = kNotMounted;
  }

 private:
  friend class Filesystem;

  Mount(Status error) : mount_(nullptr), status_(error), read_only_(false) {}
  Mount(std::shared_ptr<MountImpl> impl) : mount_(impl), status_(kOk) {}

  std::shared_ptr<MountImpl> mount_;
  Status status_;
  bool read_only_;
};

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
  // mounted, and does not cause it to be mounted. Some implementations may not
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

std::unique_ptr<DirectoryImpl> DirectoryError(Status error);
std::unique_ptr<MultipassInputStream> InputError(Status error);
std::unique_ptr<OutputStream> OutputError(Status error);

const char* GetFileName(const char* path);

}  // namespace roo_io
