#include "roo_io/fs/directory.h"

namespace roo_io {

bool Directory::close() {
  if (status_ == kClosed) return true;
  if (status_ != kOk) return false;
  dir_->close();
  status_ = dir_->status();
  dir_ = nullptr;
  return (status_ == kOk || status_ == kClosed);
}

const char* Directory::path() const {
  return status_ != kOk ? "" : dir_->path();
}

const char* Directory::name() const {
  return status_ != kOk ? "" : dir_->name();
}

void Directory::rewind() {
  if (!ok()) return;
  dir_->rewind();
  status_ = dir_->status();
}

Directory::Entry Directory::read() {
  if (!ok()) return Entry();
  auto result = dir_->read();
  status_ = dir_->status();
  return result;
}

}  // namespace roo_io