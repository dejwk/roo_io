#pragma once

#include <memory>

#include "roo_io/fs/directory_impl.h"
#include "roo_io/status.h"

namespace roo_io {

class Directory {
 public:
  using Entry = DirectoryImpl::Entry;

  Directory(Status status = kClosed) : status_(status) {}

  ~Directory() = default;
  Directory(Directory&& other) = default;

  Directory& operator=(Directory&& other) = default;

  const char* path() const;
  const char* name() const;

  bool ok() const { return status() == kOk; }

  bool isOpen() const;
  Status status() const;
  bool close();

  void rewind();

  Entry read();

 private:
  friend class Mount;

  Directory(std::unique_ptr<DirectoryImpl> dir)
      : dir_(std::move(dir)),
        status_(dir_ == nullptr ? kClosed : dir_->status()) {}

  std::unique_ptr<DirectoryImpl> dir_;
  Status status_;
};

}  // namespace roo_io