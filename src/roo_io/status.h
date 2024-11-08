#pragma once

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
  kMountError,
  kReadOnlyFilesystem,
  kNoSpaceLeftOnDevice,
  kNotFound,
  kFileExists,
  kDirectoryExists,
  kIsDirectory,
  kNotDirectory,
  kDirectoryNotEmpty,
  kInvalidPath,
  kOutOfMemory,
  kUnknownIOError,
  kTooManyFilesOpen,
};

const char* StatusAsString(Status status);

}
