#include "roo_io/fs/arduino/arduino_mount.h"

namespace roo_io {

ArduinoMountImpl::ArduinoMountImpl(FS& fs, std::function<void()> unmount_fn)
    : MountImpl(unmount_fn), fs_(fs) {}

bool ArduinoMountImpl::exists(const char* path) const {
  return fs_.exists(path);
}

Status ArduinoMountImpl::remove(const char* path) {
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  fs::File f = fs_.open(path);
  if (!f) return kNotFound;
  if (f.isDirectory()) return kInvalidType;
  return fs_.remove(path) ? kOk : kUnknownIOError;
}

bool ArduinoMountImpl::rename(const char* pathFrom, const char* pathTo) {
  return fs_.rename(pathFrom, pathTo);
}

bool ArduinoMountImpl::mkdir(const char* path) { return fs_.mkdir(path); }

bool ArduinoMountImpl::rmdir(const char* path) { return fs_.rmdir(path); }

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