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

const char* File::path() { return status_ != kOk ? "" : file_->path(); }

const char* File::name() { return status_ != kOk ? "" : file_->name(); }

bool File::isDirectory() const {
  return status_ != kOk ? false : file_->isDirectory();
}

void File::rewindDirectory() {
  if (file_ != nullptr) file_->rewindDirectory();
}

File File::openNextFile() {
  return file_ == nullptr ? File() : File(file_->openNextFile());
}

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