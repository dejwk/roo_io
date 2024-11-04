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

  virtual bool exists(const char* path) const = 0;

  virtual bool remove(const char* path) = 0;

  virtual bool rename(const char* pathFrom, const char* pathTo) = 0;

  virtual bool mkdir(const char* path) = 0;

  virtual bool rmdir(const char* path) = 0;

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
  Mount() : Mount(kClosed) {}

  Status status() const { return status_; }
  bool ok() const { return status_ == kOk; }

  bool exists(const char* path) {
    return mount_ == nullptr ? false : mount_->exists(path);
  }

  bool remove(const char* path) {
    return mount_ == nullptr ? false : mount_->remove(path);
  }

  bool rename(const char* pathFrom, const char* pathTo) {
    return mount_ == nullptr ? false : mount_->rename(pathFrom, pathTo);
  }

  bool mkdir(const char* path) {
    return mount_ == nullptr ? false : mount_->mkdir(path);
  }

  bool rmdir(const char* path) {
    return mount_ == nullptr ? false : mount_->rmdir(path);
  }

  File openForReading(const char* path) {
    return mount_ == nullptr ? File() : File(mount_->openForReading(path));
  }

  File openForAppend(const char* path) {
    return mount_ == nullptr ? File() : File(mount_->openForAppend(path));
  }

  File createOrReplace(const char* path) {
    return mount_ == nullptr ? File() : File(mount_->createOrReplace(path));
  }

  void close() {
    mount_ = nullptr;
    if (status_ == kOk) status_ = kClosed;
  }

 private:
  friend class Filesystem;

  Mount(Status error) : mount_(nullptr), status_(error) {}
  Mount(std::shared_ptr<MountImpl> impl) : mount_(impl), status_(kOk) {}

  std::shared_ptr<MountImpl> mount_;
  Status status_;
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
