#include "roo_io/fs/fsutil.h"

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

Status DeleteRecursively(roo_io::Mount& fs, const char* path) {
  Stat stat = fs.stat(path);
  if (!stat.exists()) return stat.status();
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

Status MkParentDirRecursively(roo_io::Mount& fs, const char* path) {
  Stat stat = fs.stat(path);
  if (stat.exists()) {
    return kDirectoryExists;
  }
  if (stat.status() != kNotFound) return stat.status();

  std::unique_ptr<char[]> path_copy(strdup(path));
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