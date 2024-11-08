#include "roo_io/fs/filesystem.h"

namespace roo_io {

namespace {

Status DeleteDirContentsRecursively(Mount& fs, Directory& dir) {
  if (!dir.ok()) return dir.status();
  dir.rewind();
  // LOG(INFO) << "Recursively deleting directory " << file.path();
  // file.rewindDirectory();
  while (true) {
    Directory::Entry entry = dir.read();
    if (!dir.ok()) return dir.status();
    if (entry.done()) break;
    Status s = DeleteRecursively(fs, entry.path());
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

Status DeleteRecursively(roo_io::Mount& fs, const char* path) {
  Stat stat = fs.stat(path);
  if (!stat.ok()) return stat.status();
  if (stat.isFile()) {
    return fs.remove(path);
  } else {
    Directory dir = fs.opendir(path);
    if (!dir.ok()) return dir.status();
    Status s = DeleteDirContentsRecursively(fs, dir);
    if (s != kOk) return s;
    return fs.rmdir(path);
  }
}

// namespace {

// int GetLongestSubdirName(const char* path) {
//   int max = 0;
//   int curr = 0;
//   int beg = 0;
//   int pos = beg;
//   while (true) {
//     while (path[pos] != 0 && path[pos] != '/') {
//       ++curr;
//       ++pos;
//       if (curr > max) max = curr;
//     }
//     if (path[pos] == 0) return max;
//     ++pos;
//     beg = pos;
//     curr = 0;
//   }
// }

// }

Status MkDirRecursively(roo_io::Mount& fs, const char* path) {
  Stat stat = fs.stat(path);
  if (!stat.ok()) return stat.status();
  if (stat.exists()) {
    if (stat.isFile()) {
      return kNotDirectory;
    } else {
      return kDirectoryExists;
    }
  }

  std::unique_ptr<char[]> path_copy(strdup(path));
  char* p = path_copy.get();
  while (true) {
    while (*p == '/') ++p;
    while (*p != 0 && *p != '/') ++p;
    bool end = (*p == 0);
    if (!end) {
      // Temporarily truncate.
      *p = 0;
    }
    Status s = fs.mkdir(path_copy.get());
    if (s != kOk && s != kDirectoryExists) return s;
    if (end) return kOk;
    *p = '/';
  }
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

std::unique_ptr<RandomAccessInputStream> InputError(Status error) {
  return std::unique_ptr<RandomAccessInputStream>(new NullInputStream(error));
}

std::unique_ptr<OutputStream> OutputError(Status error) {
  return std::unique_ptr<OutputStream>(new NullOutputStream(error));
}

}  // namespace roo_io