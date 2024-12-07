#pragma once

#include <memory>

#include "roo_io/status.h"

namespace roo_io {

class DirectoryImpl;

// Represent a browsable directory. A directory is like a multipass iterator
// over entries.
class Directory {
 public:
  // Represents a single directory entry. Entries are transient during directory
  // iteration.
  class Entry {
   public:
    Entry(const Entry&) = delete;
    Entry(Entry&&) = default;
    Entry& operator=(Entry&&) = default;

    // Returns the absolute path of the file or directory represented by this
    // entry.
    const char* path() const { return path_; }

    // Returns a name of the file or directory represented by this entry,
    // relative to its parent directory.
    const char* name() const { return name_; }

    // Returns true if this entry represents a directory.
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

  // Creates a directory object with the specified status (default closed).
  Directory(Status status = kClosed) : status_(status) {}

  ~Directory() = default;
  Directory(Directory&& other) = default;

  Directory& operator=(Directory&& other) = default;

  // Returns the absolute path of this directory. Empty if closed.
  const char* path() const;

  // Returns the name of this directory, relative to its parent. Empty if
  // closed.
  const char* name() const;

  // Returns true if the directory object represents an existing, open
  // directory.
  bool isOpen() const { return (status() == kOk || status() == kEndOfStream); }

  // Return true if opening or browsing the directory has failed, i.e. the
  // state is not one of kOk, kEndOfStream, or kClosed.
  bool failed() const { return !isOpen() && status() != kClosed; }

  // Returns the status of this directory. Can be one of:
  // * kOk, if the directory object is healthy and browsable,
  // * kClosed, if the directory was never opened, or if it was closed,
  // * kEndOfStream, if the directory has been read till the end,
  // * any error returned by mount.opendir().
  Status status() const { return status_; }

  // Closes this directory. If the state was an error, leaves it as is;
  // otherwise, changes the state to kClosed.
  void close();

  // If the directory is open, resets the entry index to the beginning, and
  // resets the state to kOk. Otherwise, does nothing.
  void rewind();

  // Reads a subsequent directory entry. Invalidates the previously read entry.
  // Returns true on success. If there is no more entries, or if error occurs,
  // returns false.
  bool read();

  // Returns the details of the last read entry. If read() was never called, or
  // if it returned false, the contents is undefined and should not be used.
  // The value gets invalidated by a subsequent call to read(). If you want to
  // rely on the contents of the entry beyond that, you need to make a copy.
  const Entry& entry() const { return entry_; }

 private:
  friend class Mount;

  Directory(std::unique_ptr<DirectoryImpl> dir);

  std::unique_ptr<DirectoryImpl> dir_;
  Status status_;

  Entry entry_;
};

}  // namespace roo_io