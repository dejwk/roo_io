#pragma once

#include <memory>

#include "roo_io/status.h"

namespace roo_io {

class DirectoryImpl;

/// Represents a browsable directory.
///
/// A directory behaves like a multipass iterator over transient entries:
/// repeated `read()` calls advance iteration, and `entry()` exposes the most
/// recently produced entry view.
class Directory {
 public:
  /// Represents one transient directory entry produced during iteration.
  ///
  /// The path and name pointers refer to storage owned by the directory handle
  /// and should be copied if they need to outlive the current entry view.
  class Entry {
   public:
    /// Non-copyable because entry storage is owned by the directory iterator.
    Entry(const Entry&) = delete;
    /// Move-constructs an entry view.
    Entry(Entry&&) = default;
    /// Move-assigns an entry view.
    Entry& operator=(Entry&&) = default;

    /// Returns the absolute path of the file or directory represented here.
    const char* path() const { return path_; }

    /// Returns the entry name relative to the containing directory.
    const char* name() const { return name_; }

    /// Returns whether this entry represents a directory.
    bool isDirectory() const { return is_dir_; }

   private:
    Entry() : path_(nullptr), name_(nullptr), is_dir_(false) {}

    friend class Directory;
    friend class DirectoryImpl;

    void set(const char* path, int name_offset, bool is_dir);

    const char* path_;
    const char* name_;
    bool is_dir_;
  };

  /// Creates a detached directory object with the specified status.
  Directory(Status status = kClosed);

  /// Destroys the directory object, closing it when needed.
  ~Directory() = default;
  /// Move-constructs the directory handle.
  Directory(Directory&& other) = default;

  /// Move-assigns the directory handle.
  Directory& operator=(Directory&& other) = default;

  /// Returns the absolute path of this directory, or an empty string if closed.
  const char* path() const;

  // // Returns the name of this directory, relative to its parent. Empty if
  // // closed.
  // const char* name() const;

  /// Returns whether this object represents an open browsable directory.
  bool isOpen() const { return (status() == kOk || status() == kEndOfStream); }

  /// Returns whether opening or browsing the directory failed.
  bool failed() const { return !isOpen() && status() != kClosed; }

  /// Returns the current directory status.
  ///
  /// `kOk` means iteration can continue, `kEndOfStream` means all entries have
  /// been consumed, `kClosed` means the handle is detached, and any other value
  /// is the error that prevented iteration.
  Status status() const { return status_; }

  /// Closes this directory handle.
  ///
  /// Calling this explicitly is usually unnecessary because destruction also
  /// closes the handle. If the directory is already in an error state, that
  /// status is preserved. Otherwise the handle adopts the backend close status,
  /// typically `kClosed`.
  void close();

  /// Rewinds directory iteration to the beginning when the directory is open.
  ///
  /// On success this clears `kEndOfStream` and makes the next `read()` return
  /// the first entry again.
  void rewind();

  /// Advances to the next directory entry.
  ///
  /// Returns `true` only when a new entry was read. A `false` result means
  /// end-of-directory or an error; inspect `status()` to distinguish the two.
  /// A successful call invalidates the previously returned `entry()`.
  bool read();

  /// Returns the most recently read entry.
  ///
  /// This is only valid after `read()` has returned `true`. The returned
  /// reference is transient and is invalidated by the next successful
  /// `read()`.
  const Entry& entry() const { return entry_; }

 private:
  friend class Mount;

  Directory(std::unique_ptr<DirectoryImpl> dir);

  std::unique_ptr<DirectoryImpl> dir_;
  Status status_;

  Entry entry_;
};

}  // namespace roo_io