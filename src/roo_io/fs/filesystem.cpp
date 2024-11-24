#include "roo_io/fs/filesystem.h"

namespace roo_io {

MountImpl::MountResult MountImpl::Mounted(
    std::unique_ptr<MountImpl> mount_impl) {
  return MountImpl::MountResult{.status = kOk, .mount = std::move(mount_impl)};
}

MountImpl::MountResult MountImpl::MountError(Status status) {
  return MountImpl::MountResult{.status = status, .mount = nullptr};
}

Mount Filesystem::mount() {
  std::shared_ptr<MountImpl> existing = mount_.lock();
  if (existing != nullptr) {
    return Mount(existing);
  }
  MountImpl::MountResult mount_result = mountImpl([this]() { unmountImpl(); });
  if (mount_result.status != kOk) {
    return Mount(mount_result.status);
  }
  auto impl = std::shared_ptr<MountImpl>(mount_result.mount.release());
  if (impl == nullptr) {
    return Mount(kMountError);
  }
  mount_ = impl;
  return Mount(impl);
}

const char* GetFileName(const char* path) {
  size_t i = 0;
  size_t pos = 0;
  char* p = (char*)path;
  while (*p) {
    i++;
    if (*p == '/' || *p == '\\') {
      pos = i;
    }
    p++;
  }
  return path + pos;
}

namespace {

class DirectoryErrorImpl : public DirectoryImpl {
 public:
  DirectoryErrorImpl(Status status) : status_(status) {}

  const char* path() const override { return nullptr; }
  const char* name() const override { return nullptr; }

  bool isOpen() const override { return false; }
  Status status() const override { return status_; }
  bool close() override { return true; }
  void rewind() override {}
  Entry read() override { return Entry(); }

 private:
  Status status_;
};

}  // namespace

std::unique_ptr<DirectoryImpl> DirectoryError(Status error) {
  return std::unique_ptr<DirectoryImpl>(new DirectoryErrorImpl(error));
}

std::unique_ptr<MultipassInputStream> InputError(Status error) {
  return std::unique_ptr<MultipassInputStream>(new NullInputStream(error));
}

std::unique_ptr<OutputStream> OutputError(Status error) {
  return std::unique_ptr<OutputStream>(new NullOutputStream(error));
}

}  // namespace roo_io