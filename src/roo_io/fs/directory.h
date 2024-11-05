#pragma once

#include <inttypes.h>

#include <memory>

#include "roo_io/status.h"

namespace roo_io {

class DirectoryImpl {
 public:
  ~DirectoryImpl() = default;

  virtual const char* path() const = 0;
  virtual const char* name() const = 0;

  virtual bool isOpen() const = 0;
  virtual Status status() const = 0;
  virtual bool close() = 0;

  virtual void rewind() = 0;
  virtual const char* next() = 0;

 protected:
  DirectoryImpl() = default;
};

class Directory {
 public:
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

  const char* next();

 private:
  friend class Mount;

  Directory(std::unique_ptr<DirectoryImpl> dir)
      : dir_(std::move(dir)),
        status_(dir_ == nullptr ? kClosed : dir_->status()) {}

  std::unique_ptr<DirectoryImpl> dir_;
  Status status_;
};

}  // namespace roo_io