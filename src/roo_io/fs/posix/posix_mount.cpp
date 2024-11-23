#include "roo_io/fs/posix/posix_mount.h"

#if (defined(ESP32) || defined(__linux__))

#include <dirent.h>
#include <stdio.h>
#include <unistd.h>

#include <cstring>

#include "roo_io/fs/posix/posix_directory.h"
#include "roo_io/fs/posix/posix_file_input_stream.h"
#include "roo_io/fs/posix/posix_file_output_stream.h"
#include "sys/stat.h"

namespace roo_io {

namespace {

std::unique_ptr<char[]> cat(const char* mount_point, const char* path) {
  std::unique_ptr<char[]> tmp(new char[strlen(path) + strlen(mount_point) + 2]);
  if (tmp != nullptr) {
    strcpy(tmp.get(), mount_point);
    strcat(tmp.get(), path);
  }
  return tmp;
}

}  // namespace

PosixMountImpl::PosixMountImpl(const char* mount_point, bool read_only,
                               std::function<void()> unmount_fn)
    : MountImpl(unmount_fn),
      mount_point_(strdup(mount_point)),
      read_only_(read_only) {}

bool PosixMountImpl::isReadOnly() const { return read_only_; }

Stat PosixMountImpl::stat(const char* path) const {
  if (mount_point_ == nullptr) return kNotMounted;
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  auto full_path = cat(mount_point_.get(), path);
  if (full_path == nullptr) return kOutOfMemory;
  struct stat st;
  if (::stat(full_path.get(), &st) == 0) {
    // Found!
    Stat::Type type = S_ISDIR(st.st_mode) ? Stat::kDir : Stat::kFile;
    uint64_t size = S_ISDIR(st.st_mode) ? 0 : st.st_size;
    return Stat(type, size);
  }
  switch (errno) {
    case ENAMETOOLONG:
      return kInvalidPath;
    case ENOENT:
      return kNotFound;
    case ENOMEM:
      return kOutOfMemory;
    case ENOTDIR:
      return kNotDirectory;
    default:
      return kUnknownIOError;
  }
}

Status PosixMountImpl::remove(const char* path) {
  if (mount_point_ == nullptr) return kNotMounted;
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  if (read_only_) return kReadOnlyFilesystem;
  auto full_path = cat(mount_point_.get(), path);
  if (full_path == nullptr) return kOutOfMemory;
  if (::unlink(full_path.get()) == 0) {
    return kOk;
  }
  switch (errno) {
    case ENAMETOOLONG:
      return kInvalidPath;
    case ENOENT:
      return kNotFound;
    case ENOTDIR:
      return kNotDirectory;
    default:
      return kUnknownIOError;
  }
}

Status PosixMountImpl::rename(const char* pathFrom, const char* pathTo) {
  if (mount_point_ == nullptr) return kNotMounted;
  if (pathFrom == nullptr || pathFrom[0] != '/') {
    return kInvalidPath;
  }
  if (pathTo == nullptr || pathTo[0] != '/') {
    return kInvalidPath;
  }
  if (read_only_) return kReadOnlyFilesystem;

  auto full_src_path = cat(mount_point_.get(), pathFrom);
  if (full_src_path == nullptr) return kOutOfMemory;
  auto full_dst_path = cat(mount_point_.get(), pathTo);
  if (full_dst_path == nullptr) return kOutOfMemory;

  if (::rename(full_src_path.get(), full_dst_path.get()) == 0) return kOk;
  switch (errno) {
    case ENOENT:
      return kNotFound;
    case EEXIST: {
      Stat s = stat(pathTo);
      if (!s.ok()) return s.status();
      return s.isDirectory() ? kDirectoryExists : kFileExists;
    }
    case EINVAL:
      return kInvalidPath;
    default:
      return kUnknownIOError;
  }
}

Status PosixMountImpl::mkdir(const char* path) {
  if (mount_point_ == nullptr) return kNotMounted;
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  if (read_only_) return kReadOnlyFilesystem;
  auto full_path = cat(mount_point_.get(), path);
  if (full_path.get() == nullptr) return kOutOfMemory;
  if (::mkdir(full_path.get(), 0777) == 0) return kOk;
  switch (errno) {
    case EEXIST:
      return kDirectoryExists;
    case ENAMETOOLONG:
      return kInvalidPath;
    case ENOENT:
      return kNotFound;
    case ENOTDIR:
      return kNotDirectory;
    case EROFS:
      return kReadOnlyFilesystem;
    default:
      return kUnknownIOError;
  }
}

Status PosixMountImpl::rmdir(const char* path) {
  if (mount_point_ == nullptr) return kNotMounted;
  if (path == nullptr || path[0] != '/') {
    return kInvalidPath;
  }
  if (read_only_) return kReadOnlyFilesystem;
  auto full_path = cat(mount_point_.get(), path);
  if (full_path.get() == nullptr) return kOutOfMemory;
  if (::rmdir(full_path.get()) == 0) return kOk;
  switch (errno) {
    case EEXIST:
    case ENOTEMPTY:
      return kDirectoryNotEmpty;
    case EINVAL:
      return kInvalidPath;
    case ENAMETOOLONG:
      return kInvalidPath;
    case ENOENT:
      return kNotFound;
    case ENOTDIR:
      return kNotDirectory;
    case EROFS:
      return kReadOnlyFilesystem;
    default:
      return kUnknownIOError;
  }
}

std::unique_ptr<DirectoryImpl> PosixMountImpl::opendir(const char* path) {
  if (mount_point_ == nullptr) return DirectoryError(kNotMounted);
  if (path == nullptr || path[0] != '/') {
    return DirectoryError(kInvalidPath);
  }

  auto full_path = cat(mount_point_.get(), path);
  if (full_path.get() == nullptr) return DirectoryError(kOutOfMemory);
  DIR* dir = ::opendir(full_path.get());
  if (dir != nullptr) {
    return std::unique_ptr<DirectoryImpl>(
        new PosixDirectoryImpl(path, dir, kOk));
  }
  switch (errno) {
    case ENAMETOOLONG:
      return DirectoryError(kInvalidPath);
    case ENOENT:
      return DirectoryError(kNotFound);
    case ENOTDIR:
      return DirectoryError(kNotDirectory);
    case ENFILE:
      return DirectoryError(kTooManyFilesOpen);
    default:
      return DirectoryError(kUnknownIOError);
  }
}

std::unique_ptr<MultipassInputStream> PosixMountImpl::fopen(
    const char* path) {
  if (mount_point_ == nullptr) return InputError(kNotMounted);
  if (path == nullptr || path[0] != '/') {
    return InputError(kInvalidPath);
  }
  auto full_path = cat(mount_point_.get(), path);
  if (full_path.get() == nullptr) return InputError(kOutOfMemory);
  FILE* f = ::fopen(full_path.get(), "r");
  if (f != nullptr) {
    return std::unique_ptr<MultipassInputStream>(
        new PosixFileInputStream(f));
  }
  switch (errno) {
    case ENAMETOOLONG:
    case EINVAL:
      return InputError(kInvalidPath);
    case ENOENT:
      return InputError(kNotFound);
    case ENOTDIR:
      return InputError(kNotDirectory);
    case ENFILE:
      return InputError(kTooManyFilesOpen);
    default:
      return InputError(kUnknownIOError);
  }
}

namespace {
const char* Policy2Mode(FileUpdatePolicy policy) {
  switch (policy) {
    case kAppendIfExists:
      return "a";
    case kTruncateIfExists:
      return "w";
    default:
      return "wx";
  }
}
}  // namespace

std::unique_ptr<OutputStream> PosixMountImpl::fopenForWrite(
    const char* path, FileUpdatePolicy update_policy) {
  if (mount_point_ == nullptr) return OutputError(kNotMounted);
  if (path == nullptr || path[0] != '/') {
    return OutputError(kInvalidPath);
  }
  auto full_path = cat(mount_point_.get(), path);
  if (full_path.get() == nullptr) return OutputError(kOutOfMemory);
  if (read_only_) {
    return OutputError(kReadOnlyFilesystem);
  }
  FILE* f = ::fopen(full_path.get(), Policy2Mode(update_policy));
  if (f != nullptr) {
    return std::unique_ptr<OutputStream>(new PosixFileOutputStream(f));
  }
  switch (errno) {
    case ENAMETOOLONG:
      return OutputError(kInvalidPath);
    case ENOENT:
      return OutputError(kNotFound);
    case ENOTDIR:
      return OutputError(kNotDirectory);
    case EISDIR:
      return OutputError(kIsDirectory);
    case ENFILE:
      return OutputError(kTooManyFilesOpen);
    case ENOMEM:
      return OutputError(kOutOfMemory);
    default:
      return OutputError(kUnknownIOError);
  }
}

}  // namespace roo_io

#endif
