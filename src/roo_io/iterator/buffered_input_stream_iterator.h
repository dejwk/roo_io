#pragma once

#include <cstring>
#include <memory>

#include "roo_io/iterator/input_iterator.h"
#include "roo_io/stream/input_stream.h"

namespace roo_io {

static const size_t kInputStreamIteratorBufferSize = 64;

class BufferedInputStreamIterator {
 public:
  BufferedInputStreamIterator() : rep_(new Rep()) {}

  BufferedInputStreamIterator(roo_io::InputStream& input)
      : rep_(new Rep(input)) {}

  byte read() { return rep_->read(); }

  size_t read(byte* buf, size_t count) { return rep_->read(buf, count); }

  void skip(size_t count) { rep_->skip(count); }
  Status status() const { return rep_->status(); }

  bool ok() const { return status() == roo_io::kOk; }
  bool eos() const { return status() == roo_io::kEndOfStream; }

  void reset(roo_io::InputStream& input) { rep_->reset(&input); }
  void reset() { rep_->reset(nullptr); }

 private:
  class Rep {
   public:
    Rep();
    Rep(roo_io::InputStream& input);
    // ~Rep();
    byte read();
    size_t read(byte* buf, size_t count);
    void skip(size_t count);
    Status status() const { return status_; }
    void reset(roo_io::InputStream* input);

   private:
    Rep(const Rep&) = delete;
    Rep(Rep&&);
    Rep& operator=(const Rep&) = delete;

    roo_io::InputStream* input_;
    byte buffer_[kInputStreamIteratorBufferSize];
    uint8_t offset_;
    uint8_t length_;
    Status status_;
  };

  // We keep the content on the heap for the following reasons:
  // * stack space is very limited, and we need some buffer cache;
  // * underlying file structures are using heap anyway;
  // * we want the stream object to be cheaply movable.
  std::unique_ptr<Rep> rep_;
};

inline BufferedInputStreamIterator::Rep::Rep()
    : input_(nullptr), offset_(0), length_(0), status_(kClosed) {}

inline BufferedInputStreamIterator::Rep::Rep(roo_io::InputStream& input)
    : input_(&input), offset_(0), length_(0), status_(input.status()) {}

inline void BufferedInputStreamIterator::Rep::reset(
    roo_io::InputStream* input) {
  input_ = input;
  offset_ = 0;
  length_ = 0;
  status_ = input != nullptr ? input->status() : kClosed;
}

inline byte BufferedInputStreamIterator::Rep::read() {
  if (offset_ < length_) {
    return buffer_[offset_++];
  }
  if (status_ != kOk) return byte{0};
  size_t len = input_->read(buffer_, kInputStreamIteratorBufferSize);
  if (len == 0) {
    offset_ = 0;
    length_ = 0;
    status_ = kEndOfStream;
    return byte{0};
  }
  offset_ = 1;
  length_ = len;
  return buffer_[0];
}

inline size_t BufferedInputStreamIterator::Rep::read(byte* buf, size_t count) {
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
  if (count >= kInputStreamIteratorBufferSize) {
    // Skip buffering; read directly into the client's buffer.
    size_t len = input_->read(buf, count);
    if (len == 0) {
      offset_ = 0;
      length_ = 0;
      status_ = input_->status();
    }
    return len;
  }
  size_t len = input_->read(buffer_, kInputStreamIteratorBufferSize);
  if (len == 0) {
    offset_ = 0;
    length_ = 0;
    status_ = input_->status();
    return 0;
  }
  length_ = len;
  if (count > length_) count = length_;
  memcpy(buf, buffer_, count);
  offset_ = count;
  return count;
}

inline void BufferedInputStreamIterator::Rep::skip(size_t count) {
  size_t remaining = (length_ - offset_);
  if (count < remaining) {
    offset_ += count;
  } else {
    offset_ = 0;
    length_ = 0;
    if (status_ != kOk) return;
    input_->skip(count - remaining);
    status_ = input_->status();
  }
}

}  // namespace roo_io
