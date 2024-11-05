#include "roo_io/fs/arduino/arduino_mount.h"

namespace roo_io {

ArduinoMountImpl::ArduinoMountImpl(FS& fs, bool read_only,
                                   std::function<void()> unmount_fn)
    : MountImpl(unmount_fn), fs_(fs), read_only_(read_only) {}

bool ArduinoMountImpl::isReadOnly() const { return read_only_; }

Stat ArduinoMountImpl::stat(const char* path) const {
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  fs::File f = fs_.open(path);
  if (!f) return Stat(Stat::kNone);
  return f.isDirectory() ? Stat(Stat::kDir) : Stat(Stat::kFile);
}

Status ArduinoMountImpl::remove(const char* path) {
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  {
    fs::File f = fs_.open(path);
    if (!f) return kNotFound;
    if (f.isDirectory()) return kIsDirectory;
  }
  if (read_only_) return kReadOnlyFilesystem;
  return fs_.remove(path) ? kOk : kUnknownIOError;
}

Status ArduinoMountImpl::rename(const char* pathFrom, const char* pathTo) {
  Stat src = stat(pathFrom);
  if (!src.ok()) return src.status();
  if (!src.exists()) return kNotFound;
  Stat dst = stat(pathTo);
  if (!dst.ok()) return dst.status();
  if (dst.exists()) return dst.isDirectory() ? kDirectoryExists : kFileExists;
  return fs_.rename(pathFrom, pathTo) ? kOk : kUnknownIOError;
}

Status ArduinoMountImpl::mkdir(const char* path) {
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  {
    fs::File f = fs_.open(path);
    if (f) {
      return f.isDirectory() ? kDirectoryExists : kFileExists;
    }
  }
  if (read_only_) return kReadOnlyFilesystem;
  return fs_.mkdir(path) ? kOk : kUnknownIOError;
}

Status ArduinoMountImpl::rmdir(const char* path) {
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  {
    fs::File f = fs_.open(path);
    if (!f) return kNotFound;
    if (!f.isDirectory()) return kNotDirectory;
    if (read_only_) return kReadOnlyFilesystem;
    if (fs_.rmdir(path)) return kOk;

    fs::File child;
    do {
      child = f.openNextFile();
    } while (child && (strcmp(child.name(), ".") == 0 ||
                       strcmp(child.name(), "..") == 0));
    if (child) return kDirectoryNotEmpty;
  }
  return kUnknownIOError;
}

std::unique_ptr<FileImpl> ArduinoMountImpl::openForReading(const char* path) {
  fs::File f = fs_.open(path, "r");
  roo_io::Status status = roo_io::kOk;
  if (!f) {
    if (!fs_.exists(path)) {
      status = roo_io::kNotFound;
    } else {
      status = roo_io::kOpenError;
    }
  }
  return std::unique_ptr<FileImpl>(new ArduinoFileImpl(std::move(f), status));
}

std::unique_ptr<FileImpl> ArduinoMountImpl::createOrReplace(const char* path) {
  fs::File f = fs_.open(path, "w", true);
  roo_io::Status status = roo_io::kOk;
  if (!f) {
    status = roo_io::kOpenError;
  }
  return std::unique_ptr<FileImpl>(new ArduinoFileImpl(std::move(f), status));
}

std::unique_ptr<FileImpl> ArduinoMountImpl::openForAppend(const char* path) {
  fs::File f = fs_.open(path, "a", false);
  roo_io::Status status = roo_io::kOk;
  if (!f) {
    if (!fs_.exists(path)) {
      status = roo_io::kNotFound;
    } else {
      status = roo_io::kOpenError;
    }
  }
  return std::unique_ptr<FileImpl>(new ArduinoFileImpl(std::move(f), status));
}

}  // namespace roo_io