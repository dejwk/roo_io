#pragma once

#include <memory>
#include <string>

#include "roo_io/core/buffered_input_stream_iterator.h"
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

   private:
    std::unique_ptr<MultipassInputStream> input_;
    BufferedInputStreamIterator itr_;
  };

  FileIterable(Filesystem& fs, const char* path)
      : fs_(fs), path_(strdup(path)) {}

  FileIterator iterator() const { return FileIterator(fs_, path_.get()); }

 private:
  Filesystem& fs_;
  std::shared_ptr<char[]> path_;
};

}  // namespace roo_io