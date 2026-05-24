#pragma once

#include <cstring>
#include <memory>

#include "roo_io/core/buffered_multipass_input_stream_iterator.h"
#include "roo_io/fs/filesystem.h"
#include "roo_logging.h"

namespace roo_io {

/// Iterable view that reopens a file and exposes it as a multipass iterator.
class FileIterable {
 public:
  /// Iterator that owns the opened file stream for one traversal.
  class FileIterator {
   public:
    /// Opens `path` from `fs` and prepares a buffered multipass iterator.
    FileIterator(Filesystem& fs, const char* path)
        : input_(fs.mount().fopen(path)), itr_(*input_) {
      if (!itr_.ok()) {
        LOG(ERROR) << "Failed to open file " << path << ": "
                   << StatusAsString(itr_.status());
      }
    }

    /// Reads and returns one byte.
    byte read() { return itr_.read(); }

    /// Reads up to `count` bytes into `buf`.
    size_t read(byte* buf, size_t count) { return itr_.read(buf, count); }

    /// Skips up to `count` bytes.
    void skip(size_t count) { itr_.skip(count); }

    /// Returns the current iterator status.
    Status status() const { return itr_.status(); }

    /// Returns the total size of the opened file.
    uint64_t size() { return itr_.size(); }

    /// Returns the current read position.
    uint64_t position() { return itr_.position(); }

    /// Rewinds the iterator to the beginning of the file.
    void rewind() { itr_.rewind(); }

    /// Seeks to `position` within the opened file.
    void seek(uint64_t position) { itr_.seek(position); }

   private:
    std::unique_ptr<MultipassInputStream> input_;
    BufferedMultipassInputStreamIterator itr_;
  };

  /// Stores the filesystem and file path used to create iterators.
  FileIterable(Filesystem& fs, const char* path)
      : fs_(fs), path_(DupPath(path)) {}

  /// Returns a fresh iterator that reopens the stored file path.
  FileIterator iterator() const { return FileIterator(fs_, path_.get()); }

 private:
  static std::shared_ptr<const char[]> DupPath(const char* path) {
    size_t len = std::strlen(path) + 1;
    std::shared_ptr<char[]> dup(new char[len]);
    std::memcpy(dup.get(), path, len);
    return dup;
  }

  Filesystem& fs_;
  std::shared_ptr<const char[]> path_;
};

}  // namespace roo_io