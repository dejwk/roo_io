#pragma once

#include <memory>

#include "roo_io/fs/directory.h"
#include "roo_io/fs/file.h"
#include "roo_io/status.h"
#include "roo_logging.h"

namespace roo_io {

class Stat {
 public:
  enum Type { kNone, kDir, kFile };

  Stat(Type type) : status_(kOk), type_(type) {}
  Stat(Status error) : status_(error), type_(kNone) {}

  Status status() const { return status_; }

  bool ok() const { return status_ == kOk; }
  bool isFile() const { return ok() && type_ == kFile; }
  bool isDirectory() const { return ok() && type_ == kDir; }
  bool exists() const { return ok() && type_ != kNone; }

 private:
  Status status_;
  Type type_;
};

class MountImpl {
 public:
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

  virtual std::unique_ptr<DirectoryImpl> dir(const char* path) = 0;

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

  bool exists(const char* path) const {
    return stat(path).exists();
  }

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

  Directory dir(const char* path) {
    return status_ != kOk ? Directory(status_) : Directory(mount_->dir(path));
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

const char* GetFileName(const char* path);

Status DeleteRecursively(roo_io::Mount& fs, const char* path);

}  // namespace roo_io
