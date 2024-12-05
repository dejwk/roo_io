#include "roo_io/fs/sdfat/sdfat_mount.h"

#include "roo_io/fs/sdfat/sdfat_directory.h"
#include "roo_io/fs/sdfat/sdfat_file_input_stream.h"
#include "roo_io/fs/sdfat/sdfat_file_output_stream.h"

namespace roo_io {

SdFatMountImpl::SdFatMountImpl(SdFs& fs, bool read_only,
                               std::function<void()> unmount_fn)
    : MountImpl(unmount_fn), fs_(fs), read_only_(read_only) {}

bool SdFatMountImpl::isReadOnly() const { return read_only_; }

Stat SdFatMountImpl::stat(const char* path) const {
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  FsFile f = fs_.open(path, O_RDONLY);
  if (!f) return Stat(kNotFound);
  return f.isDirectory() ? Stat(Stat::kDir, 0) : Stat(Stat::kFile, f.size());
}

Status SdFatMountImpl::remove(const char* path) {
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  {
    FsFile f = fs_.open(path);
    if (!f) return kNotFound;
    if (!f.isFile()) return kNotFile;
  }
  if (read_only_) return kReadOnlyFilesystem;
  return fs_.remove(path) ? kOk : kUnknownIOError;
}

Status SdFatMountImpl::rename(const char* pathFrom, const char* pathTo) {
  Stat src = stat(pathFrom);
  if (!src.exists()) {
    return src.status();
  }
  Stat dst = stat(pathTo);
  if (dst.status() != kOk) return dst.status();
  if (dst.exists()) return dst.isDirectory() ? kDirectoryExists : kFileExists;
  return fs_.rename(pathFrom, pathTo) ? kOk : kUnknownIOError;
}

Status SdFatMountImpl::mkdir(const char* path) {
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  {
    FsFile f = fs_.open(path);
    if (f) {
      return f.isDirectory() ? kDirectoryExists : kFileExists;
    }
  }
  if (read_only_) return kReadOnlyFilesystem;
  return fs_.mkdir(path) ? kOk : kUnknownIOError;
}

Status SdFatMountImpl::rmdir(const char* path) {
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  {
    FsFile f = fs_.open(path);
    if (!f) return kNotFound;
    if (!f.isDirectory()) return kNotDirectory;
    if (read_only_) return kReadOnlyFilesystem;
    if (fs_.rmdir(path)) return kOk;

    FsFile child;
    // do {
    child = f.openNextFile();
    // } while (child && (strcmp(child.name(), ".") == 0 ||
    //                    strcmp(child.name(), "..") == 0));
    if (child) return kDirectoryNotEmpty;
  }
  return kUnknownIOError;
}

std::unique_ptr<DirectoryImpl> SdFatMountImpl::opendir(const char* path) {
  FsFile f = fs_.open(path, O_RDONLY);
  if (!f) {
    if (!fs_.exists(path)) {
      return DirectoryError(kNotFound);
    } else {
      return DirectoryError(kOpenError);
    }
  }
  if (!f.isDirectory()) {
    return DirectoryError(kNotDirectory);
  }
  return std::unique_ptr<DirectoryImpl>(
      new SdFatDirectoryImpl(path, std::move(f), kOk));
}

std::unique_ptr<MultipassInputStream> SdFatMountImpl::fopen(
    const char* path) {
  FsFile f = fs_.open(path, O_RDONLY);
  if (!f.isOpen()) {
    if (!(fs_.exists(path))) {
      return InputError(kNotFound);
    } else {
      return InputError(kOpenError);
    }
  }
  return std::unique_ptr<MultipassInputStream>(
      new SdFatFileInputStream(std::move(f)));
}

std::unique_ptr<OutputStream> SdFatMountImpl::fopenForWrite(
    const char* path, FileUpdatePolicy update_policy) {
  if (read_only_) {
    return OutputError(kReadOnlyFilesystem);
  }
  FsFile f;
  if (update_policy == kFailIfExists) {
    f = fs_.open(path);
    if (f) {
      return OutputError(f.isDirectory() ? kDirectoryExists : kFileExists);
    }
    f = fs_.open(path, O_WRONLY | O_CREAT | O_TRUNC);
  } else {
    // Try to just open, but if it fails, check if not a directory to
    // return a more specific error.
    oflag_t oflag = O_WRONLY | O_CREAT;
    if (update_policy == kTruncateIfExists) {
      oflag |= O_TRUNC;
    } else {
      oflag |= (O_APPEND | O_AT_END);
    }

    f = fs_.open(path, oflag);
    if (!f) {
      f = fs_.open(path);
      if (!f.isFile()) {
        return OutputError(kNotFile);
      }
      return OutputError(kOpenError);
    }
  }
  if (!f) {
    return OutputError(kOpenError);
  }
  return std::unique_ptr<OutputStream>(new SdFatFileOutputStream(std::move(f)));
}

}  // namespace roo_io