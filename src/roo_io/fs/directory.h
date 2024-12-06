#pragma once

#include <memory>

#include "roo_io/fs/directory_impl.h"
#include "roo_io/status.h"

namespace roo_io {

// Represent a browsable directory.
class Directory {
 public:
  using Entry = DirectoryImpl::Entry;

  // Creates a directory object with the specified status (default closed).
  Directory(Status status = kClosed) : status_(status) {}

  ~Directory() = default;
  Directory(Directory&& other) = default;

  Directory& operator=(Directory&& other) = default;

  const char* path() const;
  const char* name() const;

  // Returns true if the directory object represents an existing, open directory.
  bool ok() const { return status() == kOk; }

  Status status() const { return status_; }

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