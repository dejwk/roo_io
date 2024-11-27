#pragma once

#include <memory>
#include <string>

#include "roo_io/fs/filesystem.h"
#include "roo_io/iterator/buffered_input_stream_iterator.h"

namespace roo_io {

class FileIterable {
 public:
  class FileIterator {
   public:
    FileIterator(Filesystem& fs, const char* path)
        : mount_(fs.mount()), input_(mount_.fopen(path)), itr_(*input_) {}

    byte read() { return itr_.read(); }

    int read(byte* buf, unsigned int count) { return itr_.read(buf, count); }

    void skip(unsigned int count) { itr_.skip(count); }

    Status status() const { return itr_.status(); }

   private:
    Mount mount_;
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