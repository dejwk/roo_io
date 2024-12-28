
#include "roo_io/fs/posix/posix_file_input_stream.h"

#include <sys/stat.h>

namespace roo_io {

PosixFileInputStream::PosixFileInputStream(Status error)
    : file_(nullptr), status_(error) {}

PosixFileInputStream::PosixFileInputStream(std::shared_ptr<MountImpl> mount,
                                           FILE* file)
    : mount_(std::move(mount)),
      file_(file),
      status_(file_ != nullptr ? kOk : kClosed) {}

PosixFileInputStream::~PosixFileInputStream() {
  if (file_ != nullptr) ::fclose(file_);
}

size_t PosixFileInputStream::read(byte* buf, size_t count) {
  if (status_ != kOk) return 0;
  int result = fread(buf, 1, count, file_);
  if (result > 0) return result;
  if (ferror(file_) != 0) {
    switch (errno) {
      case ENOMEM: {
        status_ = kOutOfMemory;
        break;
      }
      default:
        status_ = kUnknownIOError;
        break;
    }
    return 0;
  }
  if (feof(file_) != 0) {
    status_ = kEndOfStream;
  } else {
    status_ = kUnknownIOError;
    mount_.reset();
  }
  return 0;
}

void PosixFileInputStream::seek(uint64_t offset) {
  if (status_ != kOk && status_ != kEndOfStream) return;
  if (::fseek(file_, offset, SEEK_SET) == 0) {
    status_ = kOk;
    return;
  }
  switch (errno) {
    case EFBIG:
    case EINVAL:
    case EOVERFLOW:
      status_ = kSeekError;
      break;
    default:
      status_ = kUnknownIOError;
      mount_.reset();
      break;
  }
}

void PosixFileInputStream::skip(uint64_t count) {
  if (status_ != kOk) return;
  if (::fseek(file_, count, SEEK_CUR) == 0) {
    if (position() > size()) status_ = kEndOfStream;
    return;
  }
  switch (errno) {
    case EFBIG:
    case EINVAL:
    case EOVERFLOW:
      status_ = kSeekError;
      mount_.reset();
      break;
    default:
      status_ = kUnknownIOError;
      mount_.reset();
      break;
  }
}

uint64_t PosixFileInputStream::size() {
  if (status_ != kOk && status_ != kEndOfStream) return 0;
  struct stat st;
  if (::fstat(fileno(file_), &st) != 0) {
    status_ = kUnknownIOError;
    mount_.reset();
    return 0;
  }
  return st.st_size;
}

bool PosixFileInputStream::isOpen() const {
  return status_ == kOk || status_ == kEndOfStream;
}

void PosixFileInputStream::close() {
  mount_.reset();
  if (status_ != kOk && status_ != kEndOfStream) return;
  if (::fclose(file_) == 0) {
    status_ = kClosed;
    file_ = nullptr;
    return;
  }
  switch (errno) {
    case ENOSPC:
      status_ = kNoSpaceLeftOnDevice;
      break;
    default:
      status_ = kUnknownIOError;
  }
}

}  // namespace roo_io