#pragma once

#include <memory>
#include <dirent.h>
#include <string>

#include "roo_io/fs/directory.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

class PosixDirectoryImpl : public DirectoryImpl {
 public:
  PosixDirectoryImpl(const char* path, DIR* dir, Status status)
      : path_(path), dir_(dir), status_(status) {}

  bool close() override {
    if (dir_ == nullptr) return true;
    int result = ::closedir(dir_);
    if (status_ == kOk || status_ == kEndOfStream) {
      status_ = (result == 0 ? kClosed : kUnknownIOError);
    }
    return result == 0;
  }

  const char* path() const override { return path_.c_str(); }

  const char* name() const override { return GetFileName(path_.c_str()); }

  Status status() const override { return status_; }

  void rewind() override {
    if (status_ != kOk && status_ != kEndOfStream) return;
    ::rewinddir(dir_);
    status_ = kOk;
    next_ = nullptr;
  }

  Entry read() override {
    if (status_ != kOk) return Entry();
    next_ = ::readdir(dir_);
    if (next_ == nullptr) {
      status_ = kEndOfStream;
      return Entry();
    }
    file_ = path_;
    file_ += '/';
    file_.append(next_->d_name);
    return DirectoryImpl::CreateEntry(
        file_.c_str(), path_.size() + 1,
        next_->d_type == DT_DIR);
  }

 private:
  DIR* dir_;
  Status status_;
  std::string path_;
  std::string file_;
  struct dirent* next_;
};

}  // namespace roo_io