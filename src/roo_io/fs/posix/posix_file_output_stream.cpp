#include "roo_io/fs/posix/config.h"

#if ROO_IO_FS_SUPPORT_POSIX

#include "roo_io/fs/posix/posix_file_output_stream.h"

namespace roo_io {

PosixFileOutputStream::PosixFileOutputStream(Status error)
    : file_(nullptr), size_(-1), status_(error) {}

PosixFileOutputStream::PosixFileOutputStream(std::shared_ptr<MountImpl> mount,
                                             FILE* file)
    : mount_(std::move(mount)),
      file_(file),
      size_(-1),
      status_(file_ != nullptr ? kOk : kClosed) {}

PosixFileOutputStream::~PosixFileOutputStream() { close(); }

size_t PosixFileOutputStream::write(const byte* buf, size_t count) {
  if (status_ != kOk) return 0;
  size_t result = ::fwrite(buf, 1, count, file_);
  if (result == count) return result;
  if (ferror(file_)) {
    mount_.reset();
    switch (errno) {
      case ENOMEM: {
        status_ = kOutOfMemory;
        break;
      }
      case ENOSPC: {
        status_ = kNoSpaceLeftOnDevice;
        break;
      }
      default:
        status_ = kUnknownIOError;
        break;
    }
  } else {
    status_ = kUnknownIOError;
  }
  return result;
}

void PosixFileOutputStream::close() {
  if (file_ == nullptr) return;
  const int result = ::fclose(file_);
  const int close_error = errno;
  file_ = nullptr;
  mount_.reset();
  if (status_ != kOk && status_ != kEndOfStream) return;
  if (result == 0) {
    status_ = kClosed;
    return;
  }
  switch (close_error) {
    case ENOSPC:
      status_ = kNoSpaceLeftOnDevice;
      break;
    default:
      status_ = kUnknownIOError;
  }
}

}  // namespace roo_io

#endif  // ROO_IO_FS_SUPPORT_POSIX
