#include "roo_io/fs/directory.h"

#include "roo_io/fs/directory_impl.h"

namespace roo_io {

void Directory::close() {
  if (status_ == kClosed) return;
  if (status_ != kOk && status_ != kEndOfStream) return;
  dir_->close();
  if (isOpen()) {
    status_ = dir_->status();
  }
}

const char* Directory::path() const {
  return status_ != kOk ? "" : dir_->path();
}

// const char* Directory::name() const {
//   return status_ != kOk ? "" : dir_->name();
// }

void Directory::rewind() {
  if (!isOpen()) return;
  dir_->rewind();
  status_ = dir_->status();
}

bool Directory::read() {
  if (status() != kOk) return false;
  bool result = dir_->read(entry_);
  if (!result) {
    status_ = dir_->status();
  }
  return result;
}

Directory::Directory(std::unique_ptr<DirectoryImpl> dir)
    : dir_(std::move(dir)),
      status_(dir_ == nullptr ? kClosed : dir_->status()) {}

void Directory::Entry::set(const char* path, int name_offset, bool is_dir) {
  path_ = path;
  name_ = path + name_offset;
  is_dir_ = is_dir;
}

}  // namespace roo_io
