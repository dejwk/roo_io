#pragma once

#include <dirent.h>

#include <memory>
#include <string>

#include "roo_io/fs/directory.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

class PosixDirectoryImpl : public DirectoryImpl {
 public:
  PosixDirectoryImpl(std::shared_ptr<MountImpl> mount, const char* path,
                     DIR* dir, Status status)
      : mount_(std::move(mount)), path_(path), dir_(dir), status_(status) {}

  bool close() override {
    mount_.reset();
    if (dir_ == nullptr) return true;
    int result = ::closedir(dir_);
    if (status_ == kOk || status_ == kEndOfStream) {
      status_ = (result == 0 ? kClosed : kUnknownIOError);
    }
    return result == 0;
  }

  const char* path() const override { return path_.c_str(); }

  // const char* name() const override { return GetFileName(path_.c_str()); }

  Status status() const override { return status_; }

  void rewind() override {
    if (status_ != kOk && status_ != kEndOfStream) return;
    ::rewinddir(dir_);
    status_ = kOk;
    next_ = nullptr;
  }

  bool read(Directory::Entry& entry) override {
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

 private:
  std::shared_ptr<MountImpl> mount_;
  DIR* dir_;
  Status status_;
  std::string path_;
  std::string file_;
  struct dirent* next_;
};

}  // namespace roo_io