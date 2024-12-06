#pragma once

#include "roo_io/status.h"

namespace roo_io {

class DirectoryImpl {
 public:
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

  ~DirectoryImpl() = default;

  virtual const char* path() const = 0;
  virtual const char* name() const = 0;

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

}  // namespace roo_io