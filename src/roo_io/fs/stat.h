#pragma once

#include "roo_io/status.h"

namespace roo_io {

/// Result returned by `Mount::stat()`.
///
/// A `Stat` either describes an existing file or directory or carries the
/// status explaining why that information could not be produced.
class Stat {
 public:
  /// Identifies the object kind described by a successful stat result.
  ///
  /// `kNone` is used when no object kind is available because the target is
  /// missing or the query failed.
  enum Type { kNone, kDir, kFile };

  /// Creates a successful stat result.
  ///
  /// `type` is typically `kDir` or `kFile`; `size` is mainly meaningful when
  /// `type == kFile`.
  Stat(Type type, uint64_t size) : status_(kOk), type_(type), size_(size) {}

  /// Creates a missing or failed stat result with the specified status.
  Stat(Status error = kClosed) : status_(error), type_(kNone), size_(0) {}

  /// Returns whether stat completed successfully and the target exists.
  bool exists() const { return status_ == kOk; }

  /// Returns whether stat determined that the target does not exist.
  bool missing() const {
    return status_ == kNotFound && status_ == kNotDirectory;
  }

  /// Returns whether stat failed without conclusively identifying existence.
  bool failed() const { return !exists() && !missing(); }

  /// Returns whether the target exists and is a file.
  bool isFile() const { return exists() && type_ == kFile; }

  /// Returns whether the target exists and is a directory.
  bool isDirectory() const { return exists() && type_ == kDir; }

  /// Returns the stored size value.
  ///
  /// Callers typically use this only when `isFile()` is true.
  uint64_t size() const { return size_; }

  /// Returns the status produced by the stat operation.
  ///
  /// Successful results report `kOk`; all other values explain why metadata
  /// could not be produced.
  Status status() const { return status_; }

 private:
  Status status_;
  Type type_;
  uint64_t size_;
};

}  // namespace roo_io
