#pragma once

#include <inttypes.h>

#include <memory>

#include "roo_io/status.h"

namespace roo_io {

class Entry {
 public:
  Entry() : path_(nullptr), name_(nullptr), is_dir_(false) {}

  bool done() const { return path_ == nullptr; }
  const char* path() const { return path_; }
  const char* name() const { return name_; }
  bool isDirectory() const { return is_dir_; }

 private:
  friend class Directory;
  friend class DirectoryImpl;

  Entry(const char* path, int name_offset, bool is_dir)
      : path_(path), name_(path + name_offset), is_dir_(is_dir) {}

  const char* path_;
  const char* name_;
  bool is_dir_;
};

class DirectoryImpl {
 public:
  ~DirectoryImpl() = default;

  virtual const char* path() const = 0;
  virtual const char* name() const = 0;

  virtual bool isOpen() const = 0;
  virtual Status status() const = 0;
  virtual bool close() = 0;

  virtual void rewind() = 0;
  virtual Entry read() = 0;

 protected:
  DirectoryImpl() = default;

  static Entry CreateEntry(const char* path, int name_offset, bool is_dir) {
    return Entry(path, name_offset, is_dir);
  }
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