#include "roo_io/status.h"

namespace roo_io {

const char* StatusAsString(Status status) {
  switch (status) {
    case kOk:
      return "OK";
    case kEndOfStream:
      return "end of stream";
    case kClosed:
      return "file closed";
    case kOpenError:
      return "error opening file";
    case kReadError:
      return "read error";
    case kSeekError:
      return "seek error";
    case kWriteError:
      return "write error";
    case kMountError:
      return "mount error";
    case kNoSpaceLeftOnDevice:
      return "device full";
    case kNotFound:
      return "not found";
    case kAlreadyExists:
      return "alrady exists";
    case kNotDirectory:
      return "not a directory";
    case kUnknownError:
    default:
      return "unknown I/O error";
  }
}

}  // namespace roo_io