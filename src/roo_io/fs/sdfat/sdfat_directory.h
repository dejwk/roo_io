#pragma once

#include <memory>

#include "SdFat.h"
#include "roo_io/fs/directory.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

class SdFatDirectoryImpl : public DirectoryImpl {
 public:
  SdFatDirectoryImpl(const char* path, FsFile file, Status status)
      : file_(std::move(file)),
        path_(strdup(path)),
        filename_(GetFileName(path_.get())),
        entry_(),
        next_path_(new char[256]),
        status_(status) {
    strncpy(next_path_.get(), path_.get(), 256);
    int pos = strlen(path);
    if (pos < 255) {
      next_path_[pos] = '/';
      next_path_[pos + 1] = 0;
      next_path_file_offset_ = pos + 1;
    } else {
      next_path_file_offset_ = 256;
    }
  }

  bool close() override {
    entry_.close();
    file_.close();
    if (status_ = kOk) status_ = kClosed;
    return true;
  }

  const char* path() const override { return path_.get(); }

  // const char* name() const override { return filename_; }

  Status status() const override { return status_; }

  void rewind() override {
    if (status_ != kOk) return;
    file_.rewindDirectory();
    if (!file_) status_ = kClosed;
    next_path_[next_path_file_offset_] = 0;
  }

  bool read(Directory::Entry& entry) override {
    if (status_ != kOk) return false;
    if (!file_) {
      status_ = kClosed;
      return false;
    }
    entry_ = file_.openNextFile();
    if (!entry_) return false;
    entry_.getName(&next_path_[next_path_file_offset_],
                   256 - next_path_file_offset_);
    setEntry(entry, next_path_.get(), next_path_file_offset_,
             entry_.isDirectory());
    return true;
  }

 private:
  mutable FsFile file_;
  std::unique_ptr<char[]> path_;
  const char* filename_;
  FsFile entry_;
  std::unique_ptr<char[]> next_path_;
  int next_path_file_offset_;
  Status status_;
};

}  // namespace roo_io