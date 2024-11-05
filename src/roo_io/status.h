#pragma once

namespace roo_io {

enum Status {
  kOk = 0,
  kEndOfStream = 1,
  kClosed = 2,
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
  kInvalidType,
  kInvalidPath,
  kOutOfMemory,
  kUnknownIOError,
};

const char* StatusAsString(Status status);

}
