#pragma once

#include "roo_io/fs/directory.h"
#include "roo_io/status.h"

namespace roo_io {

class DirectoryImpl {
 public:
  virtual ~DirectoryImpl() = default;

  virtual const char* path() const = 0;
  // virtual const char* name() const = 0;

  virtual Status status() const = 0;

  virtual bool close() = 0;

  virtual void rewind() = 0;
  virtual bool read(Directory::Entry& entry) = 0;

 protected:
  static void setEntry(Directory::Entry& entry, const char* path,
                       int name_offset, bool is_dir) {
    entry.set(path, name_offset, is_dir);
  }

  DirectoryImpl() = default;
};

}  // namespace roo_io