#include "roo_io/fs/fsutil.h"

#include <cstring>

namespace roo_io {

namespace {

Status DeleteDirContentsRecursively(Mount& fs, Directory& dir) {
  while (dir.read()) {
    Status s = DeleteRecursively(fs, dir.entry().path());
    if (s != kOk) return s;
  }
  return dir.isOpen() ? kOk : dir.status();
}

}  // namespace

Status DeleteRecursively(roo_io::Mount& fs, const char* path) {
  Stat stat = fs.stat(path);
  if (!stat.exists()) return stat.status();
  if (stat.isFile()) {
    return fs.remove(path);
  } else {
    Directory dir = fs.opendir(path);
    if (!dir.isOpen()) return dir.status();
    Status s = DeleteDirContentsRecursively(fs, dir);
    if (s != kOk) return s;
    return fs.rmdir(path);
  }
}

Status MkDirRecursively(roo_io::Mount& fs, const char* path) {
  Stat stat = fs.stat(path);
  if (stat.exists()) {
    if (stat.isFile()) {
      return kNotDirectory;
    } else {
      return kDirectoryExists;
    }
  }
  if (stat.status() != kNotFound) return stat.status();

  auto len = std::strlen(path) + 1;
  std::unique_ptr<char[]> path_copy(new char[len]);
  std::memcpy(path_copy.get(), path, len);
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

Status MkParentDirRecursively(roo_io::Mount& fs, const char* path) {
  Stat stat = fs.stat(path);
  if (stat.exists()) {
    return kDirectoryExists;
  }
  if (stat.status() != kNotFound) return stat.status();

  auto len = std::strlen(path) + 1;
  std::unique_ptr<char[]> path_copy(new char[len]);
  std::memcpy(path_copy.get(), path, len);
  char* p = path_copy.get();
  while (true) {
    while (*p == '/') ++p;
    while (*p != 0 && *p != '/') ++p;
    if (*p == 0) return kOk;
    *p = 0;
    Status s = fs.mkdir(path_copy.get());
    if (s != kOk && s != kDirectoryExists) return s;
    *p = '/';
  }
}

MultipassInputStreamReader OpenDataFile(roo_io::Mount& fs, const char* path) {
  return MultipassInputStreamReader(fs.fopen(path));
}

OutputStreamWriter OpenDataFileForWrite(
    roo_io::Mount& fs, const char* path,
    roo_io::FileUpdatePolicy update_policy) {
  return OutputStreamWriter(fs.fopenForWrite(path, update_policy));
}

}  // namespace roo_io