#include "roo_io/fs/filesystem.h"

namespace roo_io {

Mount Filesystem::mount() {
  MountingPolicy policy = mountingPolicy();
  if (policy == kMountDisabled) {
    return Mount(kNotMounted);
  }
  bool read_only = (policy == kMountReadOnly);
  std::shared_ptr<MountImpl> existing = mount_.lock();
  if (existing != nullptr) {
    return Mount(existing, read_only);
  }
  MountImpl::MountResult mount_result = mountImpl([this]() { unmountImpl(); });
  if (mount_result.status != kOk) {
    Status status = mount_result.status;
    if (status == kGenericMountError && checkMediaPresence() == kMediaAbsent) {
      // Try to return a more specific status message.
      status = kNoMedia;
    }
    return Mount(status);
  }
  auto impl = std::shared_ptr<MountImpl>(mount_result.mount.release());
  if (impl == nullptr) {
    return Mount(kGenericMountError);
  }
  mount_ = impl;
  if (unmounting_policy_ == kUnmountLazily) {
    lazy_unmount_ = impl;
  }
  return Mount(impl, read_only);
}

void Filesystem::setUnmountingPolicy(UnmountingPolicy unmounting_policy) {
  unmounting_policy_ = unmounting_policy;
  if (unmounting_policy_ == kUnmountEagerly) {
    lazy_unmount_.reset();
  } else {
    lazy_unmount_ = mount_.lock();
  }
}

void Filesystem::forceUnmount() {
  auto m = mount_.lock();
  if (m != nullptr) {
    m->deactivate();
  }
  mount_.reset();
  lazy_unmount_.reset();
  unmountImpl();
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

}  // namespace roo_io