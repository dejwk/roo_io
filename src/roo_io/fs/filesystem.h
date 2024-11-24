#pragma once

#include <memory>

#include "roo_io/fs/directory.h"
#include "roo_io/fs/file.h"
#include "roo_io/status.h"
#include "roo_io/stream/multipass_input_stream.h"
#include "roo_logging.h"

namespace roo_io {

class Stat {
 public:
  enum Type { kNone, kDir, kFile };

  Stat(Type type, uint64_t size) : status_(kOk), type_(type), size_(size) {}
  Stat(Status error = kClosed) : status_(error), type_(kNone), size_(0) {}

  Status status() const { return status_; }

  bool ok() const { return status_ == kOk || status_ == kNotFound; }
  bool isFile() const { return ok() && type_ == kFile; }
  bool isDirectory() const { return ok() && type_ == kDir; }
  bool exists() const { return status_ == kOk; }
  uint64_t size() const { return size_; }

 private:
  Status status_;
  Type type_;
  uint64_t size_;
};

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

class Mount {
 public:
  Mount() : Mount(kNotMounted) {}

  Status status() const { return status_; }

  bool ok() const { return status_ == kOk; }

  bool exists(const char* path) const { return stat(path).exists(); }

  Stat stat(const char* path) const {
    return status_ != kOk ? Stat(status_) : mount_->stat(path);
  }

  Status remove(const char* path) {
    return status_ != kOk ? status_ : mount_->remove(path);
  }

  Status rename(const char* pathFrom, const char* pathTo) {
    return status_ != kOk ? status_ : mount_->rename(pathFrom, pathTo);
  }

  Status mkdir(const char* path) {
    return status_ != kOk ? status_ : mount_->mkdir(path);
  }

  Status rmdir(const char* path) {
    return status_ != kOk ? status_ : mount_->rmdir(path);
  }

  Directory opendir(const char* path) {
    return status_ != kOk ? Directory(status_)
                          : Directory(mount_->opendir(path));
  }

  std::unique_ptr<MultipassInputStream> fopen(const char* path) {
    return status_ != kOk ? std::unique_ptr<MultipassInputStream>(
                                new NullInputStream(status_))
                          : mount_->fopen(path);
  }

  // Always creates the file if it doesn't already exist. If the file does
  // exist, the behavior is dictated by the `update_policy`.
  std::unique_ptr<OutputStream> fopenForWrite(const char* path,
                                              FileUpdatePolicy update_policy) {
    return status_ != kOk
               ? std::unique_ptr<OutputStream>(new NullOutputStream(status_))
               : mount_->fopenForWrite(path, update_policy);
  }

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

class Filesystem {
 public:
  enum MediaPresence {
    kMediaAbsent = 0,
    kMediaPresent = 1,
    kMediaPresenceUnknown = 2
  };

  Mount mount();

  bool isMounted() const { return !mount_.expired(); }

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
