#include "roo_io/fs/filesystem.h"

namespace roo_io {

namespace {

Status DeleteDirRecursively(Mount& fs, File& file) {
  LOG(INFO) << "Recursively deleting directory " << file.path();
  file.rewindDirectory();
  while (true) {
    File f = file.openNextFile();
    if (!f.isOpen()) break;
    if (f.isDirectory()) {
      if (f.name()[0] == '.') {
        continue;
      }
      if (!DeleteDirRecursively(fs, f)) return kUnknownIOError;
    } else {
      if (!fs.remove(f.path())) {
        LOG(ERROR) << "Failed to remove " << f.path();
        return kUnknownIOError;
      }
    }
  }
  if (!fs.rmdir(file.path())) {
    LOG(ERROR) << "Failed to delete directory " << file.path();
    return kUnknownIOError;
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
  if (!fs.exists(path)) return kNotFound;
  File f = fs.openForReading(path);
  if (!f.isOpen()) {
    LOG(ERROR) << "Failed to open " << path;
    return f.status();
  }
  if (f.isDirectory()) {
    return DeleteDirRecursively(fs, f);
  } else {
    if (!fs.remove(path)) {
      LOG(ERROR) << "Failed to delete file " << path;
      return kUnknownIOError;
    }
    return kOk;
  }
}

}  // namespace roo_io