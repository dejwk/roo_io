#include "roo_io/status.h"

namespace roo_io {

const char* StatusAsString(Status status) {
  switch (status) {
    case kOk:
      return "OK";
    case kEndOfStream:
      return "end of stream";
    case kClosed:
      return "closed";
    case kNotMounted:
      return "not mounted";
    case kOpenError:
      return "failed to open";
    case kReadError:
      return "read error";
    case kSeekError:
      return "seek error";
    case kWriteError:
      return "write error";
    case kMountError:
      return "mount error";
    case kReadOnlyFilesystem:
      return "read-only filesystem";
    case kNoSpaceLeftOnDevice:
      return "device full";
    case kNotFound:
      return "not found";
    case kFileExists:
      return "file already exists";
    case kDirectoryExists:
      return "directory already exists";
    case kAncestorNotDirectory:
      return "a path component is not a directory";
    case kNotFile:
      return "destination is not a file";
    case kNotDirectory:
      return "destination is not a directory";
    case kDirectoryNotEmpty:
      return "directory not empty";
    case kInvalidPath:
      return "invalid path";
    case kOutOfMemory:
      return "out of memory";
    case kUnknownIOError:
      return "unknown I/O error";
    case kTooManyFilesOpen:
      return "too many files open";
    default:
      return "unknown error";
  }
}

}  // namespace roo_io