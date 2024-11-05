#pragma once

#include <memory>

#include "roo_io/fs/file.h"
#include "roo_io/status.h"
#include "roo_logging.h"

namespace roo_io {

class MountImpl {
 public:
  virtual ~MountImpl() {
    if (unmount_fn_ != nullptr) unmount_fn_();
  }

  virtual bool isReadOnly() const = 0;

  virtual bool exists(const char* path) const = 0;

  // Can return 'kOk', 'kNotMounted', 'kNotFound', 'kInvalidPath',
  // 'kNotFile', 'kOutOfMemory', 'kUnknownIOError'.
  virtual Status remove(const char* path) = 0;

  virtual bool rename(const char* pathFrom, const char* pathTo) = 0;

  // Can return 'kOk', 'kNotMounted', 'kDirectoryExists', 'kInvalidPath',
  // 'kInvalidType', 'kOutOfMemory', 'kUnknownIOError'.
  virtual Status mkdir(const char* path) = 0;

  virtual Status rmdir(const char* path) = 0;

  virtual std::unique_ptr<FileImpl> openForReading(const char* path) = 0;
  virtual std::unique_ptr<FileImpl> openForAppend(const char* path) = 0;
  virtual std::unique_ptr<FileImpl> createOrReplace(const char* path) = 0;

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

  bool exists(const char* path) {
    return status_ != kOk ? false : mount_->exists(path);
  }

  Status remove(const char* path) {
    return status_ != kOk ? status_ : mount_->remove(path);
  }

  bool rename(const char* pathFrom, const char* pathTo) {
    return status_ != kOk ? false : mount_->rename(pathFrom, pathTo);
  }

  Status mkdir(const char* path) {
    return status_ != kOk ? status_ : mount_->mkdir(path);
  }

  Status rmdir(const char* path) {
    return status_ != kOk ? status_ : mount_->rmdir(path);
  }

  File openForReading(const char* path) {
    return status_ != kOk ? File(status_) : File(mount_->openForReading(path));
  }

  File openForAppend(const char* path) {
    return status_ != kOk ? File(status_) : File(mount_->openForAppend(path));
  }

  File createOrReplace(const char* path) {
    return status_ != kOk ? File(status_) : File(mount_->createOrReplace(path));
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
  Mount mount();

  bool isMounted() const { return !mount_.expired(); }

 protected:
  Filesystem() = default;

  virtual std::unique_ptr<MountImpl> mountImpl(
      std::function<void()> unmount_fn) = 0;

  virtual void unmountImpl() = 0;

 private:
  std::weak_ptr<MountImpl> mount_;
};

Status DeleteRecursively(roo_io::Mount& fs, const char* path);

}  // namespace roo_io
