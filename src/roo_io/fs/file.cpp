#include "roo_io/fs/file.h"

namespace roo_io {

bool File::isOpen() const { return file_ == nullptr ? false : file_->isOpen(); }

Status File::status() const {
  return status_ != kOk ? status_ : file_->status();
}

bool File::close() {
  if (status_ == kClosed) return true;
  if (status_ != kOk) return false;
  file_->close();
  status_ = file_->status();
  return (status_ == kOk);
}

const char* File::path() const { return status_ != kOk ? "" : file_->path(); }

const char* File::name() const { return status_ != kOk ? "" : file_->name(); }

uint64_t File::size() const { return status_ != kOk ? 0 : file_->size(); }

std::unique_ptr<RandomAccessInputStream> File::asInputStream() && {
  if (status_ != kOk) {
    return std::unique_ptr<RandomAccessInputStream>(
        new NullInputStream(status_));
  }
  return std::move(*file_).asInputStream();
}

std::unique_ptr<OutputStream> File::asOutputStream() && {
  if (status_ != kOk) {
    return std::unique_ptr<OutputStream>(new NullOutputStream(status_));
  }
  return std::move(*file_).asOutputStream();
}

}  // namespace roo_io