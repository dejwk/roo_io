#include "roo_io/fs/file.h"

namespace roo_io {

bool File::isOpen() const { return file_ == nullptr ? false : file_->isOpen(); }

Status File::status() const {
  return file_ == nullptr ? kClosed : file_->status();
}

bool File::close() { return file_ == nullptr ? true : file_->close(); }

const char* File::path() { return file_ == nullptr ? "" : file_->path(); }

const char* File::name() { return file_ == nullptr ? "" : file_->name(); }

bool File::isDirectory() const {
  return file_ == nullptr ? false : file_->isDirectory();
}

void File::rewindDirectory() {
  if (file_ != nullptr) file_->rewindDirectory();
}

File File::openNextFile() {
  return file_ == nullptr ? File() : File(file_->openNextFile());
}

uint64_t File::size() const { return file_ == nullptr ? 0 : file_->size(); }

std::unique_ptr<RandomAccessInputStream> File::asInputStream() && {
  if (file_ == nullptr) {
    return std::unique_ptr<RandomAccessInputStream>(new NullInputStream());
  }
  return std::move(*file_).asInputStream();
}

std::unique_ptr<OutputStream> File::asOutputStream() && {
  if (file_ == nullptr) {
    return std::unique_ptr<OutputStream>(new NullOutputStream());
  }
  return std::move(*file_).asOutputStream();
}

}  // namespace roo_io