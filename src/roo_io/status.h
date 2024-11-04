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
  kNoSpaceLeftOnDevice,
  kNotFound,
  kAlreadyExists,
  kNotDirectory,
  kUnknownError,
};

const char* StatusAsString(Status status);

}
