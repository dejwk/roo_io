#include "roo_io/fs/posix/config.h"

#if ROO_IO_FS_SUPPORT_POSIX

#include <cerrno>

#include "roo_io/fs/posix/posix_file_output_stream.h"

namespace roo_io {
namespace {

// Maps stdio output errors to the stream's status.
Status StatusFromWriteError(int error) {
  switch (error) {
    case ENOMEM:
      return kOutOfMemory;
    case ENOSPC:
      return kNoSpaceLeftOnDevice;
    default:
      return kUnknownIOError;
  }
}

}  // namespace

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
  if (result < count) {
    status_ =
        ::ferror(file_) != 0 ? StatusFromWriteError(errno) : kUnknownIOError;
  }
  return result;
}

void PosixFileOutputStream::flush() {
  if (status_ != kOk || file_ == nullptr) return;
  if (::fflush(file_) != 0) status_ = StatusFromWriteError(errno);
}

void PosixFileOutputStream::close() {
  if (file_ == nullptr) return;
  const int result = ::fclose(file_);
  const int close_error = errno;
  file_ = nullptr;
  mount_.reset();
  if (status_ != kOk && status_ != kEndOfStream) return;
  status_ = result == 0 ? kClosed : StatusFromWriteError(close_error);
}

}  // namespace roo_io

#endif  // ROO_IO_FS_SUPPORT_POSIX
