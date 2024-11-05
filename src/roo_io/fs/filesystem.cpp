#include "roo_io/fs/filesystem.h"

namespace roo_io {

namespace {

Status DeleteDirContentsRecursively(Mount& fs, Directory& dir) {
  if (!dir.ok()) return dir.status();
  dir.rewind();
  // LOG(INFO) << "Recursively deleting directory " << file.path();
  // file.rewindDirectory();
  while (true) {
    const char* next = dir.next();
    if (!dir.ok()) return dir.status();
    if (next == nullptr) break;
    Status s = DeleteRecursively(fs, next);
    if (s != kOk) return s;
  }
  return kOk;
}

}  // namespace

Mount Filesystem::mount() {
  std::shared_ptr<MountImpl> existing = mount_.lock();
  if (existing != nullptr) {
    return Mount(existing);
  }
  auto impl = std::shared_ptr<MountImpl>(
      mountImpl([this]() { unmountImpl(); }).release());
  if (impl == nullptr) {
    return Mount(kMountError);
  }
  mount_ = impl;
  return Mount(impl);
}

Status DeleteRecursively(roo_io::Mount& fs, const char* path) {
  Stat stat = fs.stat(path);
  if (!stat.ok()) return stat.status();
  if (stat.isFile()) {
    return fs.remove(path);
  } else {
    Directory dir = fs.dir(path);
    if (!dir.ok()) return dir.status();
    Status s = DeleteDirContentsRecursively(fs, dir);
    if (s != kOk) return s;
    return fs.rmdir(path);
  }
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