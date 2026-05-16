#pragma once

#include <cstring>
#include <memory>

#include "roo_io/core/buffered_multipass_input_stream_iterator.h"
#include "roo_io/fs/filesystem.h"
#include "roo_logging.h"

namespace roo_io {

class FileIterable {
 public:
  class FileIterator {
   public:
    FileIterator(Filesystem& fs, const char* path)
        : input_(fs.mount().fopen(path)), itr_(*input_) {
      if (!itr_.ok()) {
        LOG(ERROR) << "Failed to open file " << path << ": "
                   << StatusAsString(itr_.status());
      }
    }

    byte read() { return itr_.read(); }

    size_t read(byte* buf, size_t count) { return itr_.read(buf, count); }

    void skip(size_t count) { itr_.skip(count); }

    Status status() const { return itr_.status(); }

    uint64_t size() { return itr_.size(); }

    uint64_t position() { return itr_.position(); }

    void rewind() { itr_.rewind(); }

    void seek(uint64_t position) { itr_.seek(position); }

   private:
    std::unique_ptr<MultipassInputStream> input_;
    BufferedMultipassInputStreamIterator itr_;
  };

  FileIterable(Filesystem& fs, const char* path)
      : fs_(fs), path_(DupPath(path)) {}

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