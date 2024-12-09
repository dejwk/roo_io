#pragma once

#include <FS.h>

#include <memory>

#include "roo_io/byte.h"
#include "roo_io/status.h"

namespace roo_io {

static const int kFileIteratorBufferSize = 64;

class ArduinoFileIterator {
 public:
  ArduinoFileIterator(::fs::File file) : rep_(new Rep(std::move(file))) {}

  byte read() { return rep_->read(); }

  size_t read(byte* buf, size_t count) { return rep_->read(buf, count); }

  void skip(size_t count) { rep_->skip(count); }
  Status status() const { return rep_->status(); }

  // void reset(::File file) { rep_->reset(&input); }
  // void reset() { rep_->reset(nullptr); }

 private:
  class Rep {
   public:
    Rep(::fs::File file);
    ~Rep();
    byte read();
    size_t read(byte* buf, size_t count);
    void skip(size_t count);
    Status status() const { return status_; }

   private:
    Rep(const Rep&) = delete;
    Rep(Rep&&) = delete;
    Rep& operator=(const Rep&) = delete;

    ::fs::File file_;
    byte buffer_[kFileIteratorBufferSize];
    byte offset_;
    byte length_;
    Status status_;
  };

  // We keep the content on the heap for the following reasons:
  // * stack space is very limited, and we need some buffer cache;
  // * underlying file structures are using heap anyway;
  // * we want the stream object to be cheaply movable.
  std::unique_ptr<Rep> rep_;
};

inline ArduinoFileIterator::Rep::Rep(::fs::File file)
    : file_(std::move(file)),
      offset_(0),
      length_(0),
      status_(file_ ? kOk : kClosed) {}

inline ArduinoFileIterator::Rep::~Rep() { file_.close(); }

inline byte ArduinoFileIterator::Rep::read() {
  if (offset_ < length_) {
    return buffer_[offset_++];
  }
  if (status_ != kOk) return 0;
  size_t len = file_.read(buffer_, kFileIteratorBufferSize);
  if (len == 0) {
    offset_ = 0;
    length_ = 0;
    status_ = kEndOfStream;
    return 0;
  } else if (len == ((size_t)(-1))) {
    offset_ = 0;
    length_ = 0;
    status_ = kReadError;
    return 0;
  }
  offset_ = 1;
  length_ = len;
  return buffer_[0];
}

inline size_t ArduinoFileIterator::Rep::read(byte* buf, size_t count) {
  if (offset_ < length_) {
    // Have some data still in the buffer; just return that.
    if (count > (length_ - offset_)) count = length_ - offset_;
    memcpy(buf, &buffer_[offset_], count);
    offset_ += count;
    return count;
  }
  if (status_ != kOk) {
    // Already done.
    return 0;
  }
  if (count >= kFileIteratorBufferSize) {
    // Skip buffering; read directly into the client's buffer.
    size_t len = file_.read(buf, count);
    if (len == 0) {
      offset_ = 0;
      length_ = 0;
      status_ = kEndOfStream;
      return 0;
    } else if (len == ((size_t)(-1))) {
      offset_ = 0;
      length_ = 0;
      status_ = kReadError;
      return 0;
    }
    return len;
  }
  size_t len = file_.read(buffer_, kFileIteratorBufferSize);
  if (len == 0) {
    offset_ = 0;
    length_ = 0;
    status_ = kEndOfStream;
    return 0;
  } else if (len == ((size_t)(-1))) {
    offset_ = 0;
    length_ = 0;
    status_ = kReadError;
    return 0;
  }
  length_ = len;
  if (count > length_) count = length_;
  memcpy(buf, buffer_, count);
  offset_ = count;
  return count;
}

inline void ArduinoFileIterator::Rep::skip(size_t count) {
  if (status_ != kOk) return;
  size_t remaining = (length_ - offset_);
  if (count < remaining) {
    offset_ += count;
  } else {
    offset_ = 0;
    length_ = 0;
    if (!file_.seek(count - remaining, SeekCur)) {
      status_ = kSeekError;
    }
    if (file_.position() > file_.size()) {
      status_ = kEndOfStream;
    }
  }
}

}  // namespace roo_io
