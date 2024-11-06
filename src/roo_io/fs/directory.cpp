#include "roo_io/fs/directory.h"

namespace roo_io {

bool Directory::isOpen() const { return dir_ == nullptr ? false : dir_->isOpen(); }

Status Directory::status() const {
  return status_ != kOk ? status_ : dir_->status();
}

bool Directory::close() {
  if (status_ == kClosed) return true;
  if (status_ != kOk) return false;
  dir_->close();
  status_ = dir_->status();
  dir_ = nullptr;
  return (status_ == kOk);
}

const char* Directory::path() const { return status_ != kOk ? "" : dir_->path(); }

const char* Directory::name() const { return status_ != kOk ? "" : dir_->name(); }

void Directory::rewind() {
  if (dir_ != nullptr) dir_->rewind();
}

Directory::Entry Directory::read() {
  return dir_ == nullptr ? Entry() : dir_->read();
}

}  // namespace roo_io