#pragma once

#include <functional>
#include <memory>

#include "roo_io/core/multipass_input_stream.h"
#include "roo_io/core/output_stream.h"
#include "roo_io/fs/directory_impl.h"
#include "roo_io/fs/file_update_policy.h"
#include "roo_io/fs/stat.h"
#include "roo_io/status.h"

namespace roo_io {

class Stat;

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

  virtual std::unique_ptr<DirectoryImpl> opendir(
      std::shared_ptr<MountImpl> mount, const char* path) = 0;

  virtual std::unique_ptr<MultipassInputStream> fopen(
      std::shared_ptr<MountImpl> mount, const char* path) = 0;

  virtual std::unique_ptr<OutputStream> fopenForWrite(
      std::shared_ptr<MountImpl> mount, const char* path,
      FileUpdatePolicy update_policy) = 0;

  virtual bool active() const = 0;

  // Called in case this mount gets forcefully closed. Further method calls
  // should fail with kNotMounted.
  virtual void deactivate() = 0;

 protected:
  MountImpl(std::function<void()> unmount_fn) : unmount_fn_(unmount_fn) {}

 private:
  std::function<void()> unmount_fn_;
};

// Helper functions.
std::unique_ptr<DirectoryImpl> DirectoryError(Status error);
std::unique_ptr<MultipassInputStream> InputError(Status error);
std::unique_ptr<OutputStream> OutputError(Status error);

}  // namespace roo_io
