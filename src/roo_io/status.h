#pragma once

#include "roo_logging.h"

namespace roo_io {

enum Status {
  kOk = 0,
  kEndOfStream = 1,
  kClosed,
  kNotMounted,
  kOpenError,
  kReadError,
  kSeekError,
  kWriteError,

  kGenericMountError,

  // A write operation was requested, but the filesystem is mounted as
  // read-only.
  kReadOnlyFilesystem,

  // Requested file, directory, or its path component, does not exist.
  kNotFound,

  // A create or rename operation was requested, while the destination already
  // exists and it is a file.
  kFileExists,

  // A create or rename operation was requested, while the destination already
  // exists and it is a directory.
  kDirectoryExists,

  // A component used as a directory in the path is not, in fact, a directory.
  kNotDirectory,

  // An operation was requested that expects an existing file, while the
  // destination exists but it is not a file.
  kNotFile,

  // The 'rmdir' operation was requested, while the target directory exists but
  // it is not empty.
  kDirectoryNotEmpty,

  // The specified path is syntactically invalid.
  kInvalidPath,

  // Out-of-memory error has occurred.
  kOutOfMemory,

  // The limit of open files has been reached.
  kTooManyFilesOpen,

  // The capacity of the target device has been exceeded.
  kNoSpaceLeftOnDevice,

  // Unexpected general I/O error has ocurred.
  kUnknownIOError,

  // Permissions are insufficient.
  kAccessDenied,

  // Mount failed because the media is absent.
  kNoMedia,
};

const char* StatusAsString(Status status);

inline roo_logging::Stream& operator<<(roo_logging::Stream& s, Status status) {
  s << StatusAsString(status);
  return s;
}

}  // namespace roo_io