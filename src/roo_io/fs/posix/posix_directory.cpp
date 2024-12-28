#include "roo_io/fs/posix/posix_directory.h"

namespace roo_io {

PosixDirectoryImpl::PosixDirectoryImpl(std::shared_ptr<MountImpl> mount,
                                       const char* path, DIR* dir,
                                       Status status)
    : mount_(std::move(mount)), path_(path), dir_(dir), status_(status) {}

bool PosixDirectoryImpl::close() {
  mount_.reset();
  if (dir_ == nullptr) return true;
  int result = ::closedir(dir_);
  if (status_ == kOk || status_ == kEndOfStream) {
    status_ = (result == 0 ? kClosed : kUnknownIOError);
  }
  return result == 0;
}

void PosixDirectoryImpl::rewind() {
  if (status_ != kOk && status_ != kEndOfStream) return;
  ::rewinddir(dir_);
  status_ = kOk;
  next_ = nullptr;
}

bool PosixDirectoryImpl::read(Directory::Entry& entry) {
  if (status_ != kOk) return false;
  next_ = ::readdir(dir_);
  if (next_ == nullptr) {
    status_ = kEndOfStream;
    return false;
  }
  file_ = path_;
  if (file_.empty() || file_.back() != '/') {
    file_ += '/';
  }
  file_.append(next_->d_name);
  setEntry(entry, file_.c_str(), path_.size() + 1, next_->d_type == DT_DIR);
  return true;
}

}  // namespace roo_io