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
    case kAlreadyExists:
      return "alrady exists";
    case kInvalidType:
      return "incompatible type";
    case kInvalidPath:
      return "invalid path";
    case kOutOfMemory:
      return "out of memory";
    case kUnknownIOError:
      return "unknown I/O error";
    default:
      return "unknown error";
  }
}

}  // namespace roo_io