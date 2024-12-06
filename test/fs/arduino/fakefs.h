#pragma once

#include <cstring>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "roo_io/byte.h"
#include "roo_io/status.h"
#include "roo_logging.h"

namespace roo_io {
namespace fakefs {

// A fake (in-memory) file.
class File {
 public:
  File() = default;

  size_t size() const { return data_.size(); }

  size_t read(size_t pos, byte* buf, size_t size) const;

  size_t write(size_t pos, const byte* buf, size_t size);

 private:
  friend class FileStream;

  std::vector<byte> data_;
};

class Dir;

// A fake (in-memory) directory entry, correspoinding to a file or a sub-dir.
class Entry {
 public:
  const std::string& name() const { return name_; }

  bool isFile() const { return file_ != nullptr; }
  bool isDir() const { return dir_ != nullptr; }

  File& file() { return *file_; }
  Dir& dir() { return *dir_; }

  static std::unique_ptr<Entry> DirEntry(const std::string& name);

  static std::unique_ptr<Entry> FileEntry(const std::string& name);

  bool isDescendantOf(const Entry& e) const;

 private:
  friend class Dir;

  void rename(const std::string& name) { name_ = name; }

  Entry(std::string name, std::unique_ptr<File> file)
      : name_(std::move(name)), parent_(nullptr), file_(std::move(file)) {}

  Entry(std::string name, std::unique_ptr<Dir> dir)
      : name_(std::move(name)), parent_(nullptr), dir_(std::move(dir)) {}

  void setParent(const Entry* parent) { parent_ = parent; }

  const Entry* parent() const { return parent_; }

  std::string name_;
  const Entry* parent_;

  std::unique_ptr<File> file_;
  std::unique_ptr<Dir> dir_;
};

// A fake (in-memory) directory.
class Dir {
 public:
  int entryCount() { return entries_.size(); }

  Entry* find(const std::string& name);

  void rename(const std::string& oldname, const std::string& newname);

  roo_io::Status mkdir(const Entry* parent, const std::string& name,
                       Entry** dir);

  Status create(const Entry* parent, const std::string& name, Entry** file);

  roo_io::Status rmdir(const std::string& name);

  roo_io::Status rm(const std::string& name);

  std::unique_ptr<Entry> detach(const std::string& name);

  void attach(const Entry* parent, const std::string& name,
              std::unique_ptr<Entry> entry);

 private:
  friend class DirIterator;

  std::list<std::unique_ptr<Entry>>::iterator lookup(const std::string& name);

  std::list<std::unique_ptr<Entry>> entries_;
};

// Supports going over the fake directory's entries.
class DirIterator {
 public:
  DirIterator() = default;
  DirIterator(DirIterator&& other) = default;

  void open(Dir& dir);
  void close();

  // Returns true if the iterator is open; possibly at end-of-stream.
  bool isOpen() const;

  // Returns true if the iterator points at a valid entry.
  bool ok() const;

  void rewind();

  bool next();

  Entry& entry() { return **current_; }

 private:
  //   Status status_;
  Dir* dir_;
  bool bos_;
  std::list<std::unique_ptr<Entry>>::iterator current_;
};

// Supports reading from and writing to a fake file, using stream-like
// operations.
class FileStream {
 public:
  FileStream(Status status = kClosed);

  void setError(Status status);

  void open(File* file, bool readonly);

  Status status() const { return status_; }

  void close();

  bool isOpen() const { return file_ != nullptr; }

  size_t size() { return file_->size(); }

  size_t read(byte* target, size_t size);

  size_t write(const byte* source, size_t size);

  void seek(size_t pos);

  void skip(size_t offset) { position_ += offset; }

  size_t size() const { return isOpen() ? file_->size() : 0; }
  size_t position() const { return position_; }

 private:
  File* file_;
  bool read_only_;
  size_t position_;

  Status status_;
};

struct ResolvedPath {
  Status status;
  Entry* parent;
  std::string basename;
};

struct StatResult {
  enum Type { kDir, kFile, kUnknown };

  Status status;
  Type type;
  size_t size;
};

// Fake in-memory filesystem.
class FakeFs {
 public:
  enum OpenFlags { kRead = 1, kWrite = 2, kTruncate = 4, kAppend = 8 };

  FakeFs() : root_(Entry::DirEntry("")) {}

  Entry* root() { return root_.get(); }

  StatResult stat(const char* path) const;

  Status remove(const char* path);

  Status rename(const char* pathFrom, const char* pathTo);

  Status mkdir(const char* path);

  Status rmdir(const char* path);

  Status opendir(const char* path, DirIterator& itr);

  FileStream open(const char* path, int flags);

  Status findEntryByPath(const char* name, Entry** out,
                         bool create_subdirs = false) const;

  ResolvedPath resolvePath(const char* path, bool create_subdirs = false);

 private:
  std::unique_ptr<Entry> root_;
};

Status CreateTextFile(FakeFs& fs, const char* path, const char* contents);

std::string ReadTextFile(FakeFs& fs, const char* path);

}  // namespace fakefs
}  // namespace roo_io
