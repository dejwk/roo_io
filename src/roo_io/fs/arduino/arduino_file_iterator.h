#pragma once

#include <FS.h>

#include <memory>

namespace roo_io {

static const int kFileIteratorBufferSize = 64;

class ArduinoFileIterator {
 public:
  ArduinoFileIterator(::File file) : rep_(new Rep(std::move(file))) {}

  byte read() { return rep_->read(); }
  void skip(uint32_t count) { rep_->skip(count); }

 private:
  class Rep {
   public:
    Rep(::File file);
    ~Rep();
    byte read();
    size_t read(byte* buf, int count);
    void skip(uint32_t count);

   private:
    Rep(const Rep&) = delete;
    Rep(Rep&&) = delete;
    Rep& operator=(const Rep&) = delete;

    ::File file_;
    byte buffer_[kFileIteratorBufferSize];
    byte offset_;
    byte length_;
  };

  // We keep the content on the heap for the following reasons:
  // * stack space is very limited, and we need some buffer cache;
  // * underlying file structures are using heap anyway;
  // * we want the stream object to be cheaply movable.
  std::unique_ptr<Rep> rep_;
};

inline ArduinoFileIterator::Rep::Rep(::File file)
    : file_(std::move(file)), offset_(0), length_(0) {}

inline ArduinoFileIterator::Rep::~Rep() { file_.close(); }

inline byte ArduinoFileIterator::Rep::read() {
  if (offset_ >= length_) {
    length_ = file_.read(buffer_, kFileIteratorBufferSize);
    offset_ = 0;
    if (length_ <= 0) return 0;
  }
  return buffer_[offset_++];
}

inline void ArduinoFileIterator::Rep::skip(uint32_t count) {
  uint32_t remaining = (length_ - offset_);
  if (count < remaining) {
    offset_ += count;
  } else {
    offset_ = 0;
    length_ = 0;
    count -= remaining;
    file_.seek(count, SeekCur);
  }
}

}  // namespace roo_io
