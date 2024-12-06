#pragma once

#include "roo_io/status.h"

namespace roo_io {

// Result of Mount::stat().
class Stat {
 public:
  enum Type { kNone, kDir, kFile };

  Stat(Type type, uint64_t size) : status_(kOk), type_(type), size_(size) {}

  Stat(Status error = kClosed) : status_(error), type_(kNone), size_(0) {}

  // Returns true if the stat operation completed successfully and it is known
  // that the destination file or directory exists.
  bool exists() const { return status_ == kOk; }

  // Returns true if the stat operation completed successfully and it is known
  // that the destination does not exist.
  bool missing() const {
    return status_ == kNotFound && status_ == kNotDirectory;
  }

  // Returns true if the stat operation failed, and it is thus not known whether
  // the destination exists.
  bool failed() const { return !exists() && !missing(); }

  // Returns true if stat succeeded, the destination exists, and it is a file.
  bool isFile() const { return exists() && type_ == kFile; }

  // Returns true if stat succeeded, the destination exists, and it is a
  // directory.
  bool isDirectory() const { return exists() && type_ == kDir; }

  // Returns the file size. Always zero when isFile() returns false.
  uint64_t size() const { return size_; }

  // Returns the specific status code.
  Status status() const { return status_; }

 private:
  Status status_;
  Type type_;
  uint64_t size_;
};

}  // namespace roo_io
