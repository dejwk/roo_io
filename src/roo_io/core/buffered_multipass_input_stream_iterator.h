#pragma once

#include <cstring>
#include <memory>

#include "roo_io/core/input_iterator.h"
#include "roo_io/core/multipass_input_stream.h"

namespace roo_io {

static const size_t kMultipassInputStreamIteratorBufferSize = 64;

class BufferedMultipassInputStreamIterator {
 public:
  BufferedMultipassInputStreamIterator() : rep_(new Rep()) {}

  BufferedMultipassInputStreamIterator(roo_io::MultipassInputStream& input)
      : rep_(new Rep(input)) {}

  BufferedMultipassInputStreamIterator(
      BufferedMultipassInputStreamIterator&& other) = default;

  BufferedMultipassInputStreamIterator& operator=(
      BufferedMultipassInputStreamIterator&& other) = default;

  byte read() { return rep_->read(); }

  size_t read(byte* buf, size_t count) { return rep_->read(buf, count); }

  void skip(size_t count) { rep_->skip(count); }
  Status status() const { return rep_->status(); }

  uint64_t size() const { return rep_->size(); }
  uint64_t position() const { return rep_->position(); }

  void rewind() { rep_->rewind(); }
  void seek(uint64_t position) { rep_->seek(position); }

  bool ok() const { return status() == roo_io::kOk; }
  bool eos() const { return status() == roo_io::kEndOfStream; }

  void reset(roo_io::MultipassInputStream& input) { rep_->reset(&input); }
  void reset() { rep_->reset(nullptr); }

 private:
  class Rep {
   public:
    Rep();
    Rep(roo_io::MultipassInputStream& input);
    // ~Rep();
    byte read();
    size_t read(byte* buf, size_t count);
    void skip(size_t count);

    Status status() const { return status_; }
    void reset(roo_io::MultipassInputStream* input);

    uint64_t size() const;
    uint64_t position() const;

    void rewind();
    void seek(uint64_t position);

   private:
    Rep(const Rep&) = delete;
    Rep(Rep&&);
    Rep& operator=(const Rep&);

    roo_io::MultipassInputStream* input_;
    byte buffer_[kMultipassInputStreamIteratorBufferSize];
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

inline BufferedMultipassInputStreamIterator::Rep::Rep()
    : input_(nullptr), offset_(0), length_(0), status_(kClosed) {}

inline BufferedMultipassInputStreamIterator::Rep::Rep(
    roo_io::MultipassInputStream& input)
    : input_(&input), offset_(0), length_(0), status_(input.status()) {}

inline void BufferedMultipassInputStreamIterator::Rep::reset(
    roo_io::MultipassInputStream* input) {
  input_ = input;
  offset_ = 0;
  length_ = 0;
  status_ = input != nullptr ? input->status() : kClosed;
}

inline uint64_t BufferedMultipassInputStreamIterator::Rep::size() const {
  return input_->size();
}

inline uint64_t BufferedMultipassInputStreamIterator::Rep::position() const {
  return input_->position() + offset_ - length_;
}

inline void BufferedMultipassInputStreamIterator::Rep::rewind() {
  if (status_ != kOk && status_ != kEndOfStream) return;
  uint64_t file_pos = input_->position();
  if (file_pos <= length_) {
    // Keep the buffer data and length.
    offset_ = 0;
  } else {
    // Reset the buffer.
    input_->rewind();
    offset_ = 0;
    length_ = 0;
    status_ = input_->status();
  }
}

inline void BufferedMultipassInputStreamIterator::Rep::seek(uint64_t position) {
  if (status_ != kOk && status_ != kEndOfStream) return;
  uint64_t file_pos = input_->position();
  if (file_pos <= position + length_ && file_pos >= position) {
    // Seek within the area we have in the buffer.
    offset_ = position + length_ - file_pos;
  } else {
    // Seek outside the buffer. Just seek in the file and reset the buffer.
    input_->seek(position);
    offset_ = 0;
    length_ = 0;
    status_ = input_->status();
  }
  status_ = kOk;
}

inline byte BufferedMultipassInputStreamIterator::Rep::read() {
  if (offset_ < length_) {
    return buffer_[offset_++];
  }
  if (status_ != kOk) return byte{0};
  size_t len = input_->read(buffer_, kMultipassInputStreamIteratorBufferSize);
  if (len == 0) {
    offset_ = 0;
    length_ = 0;
    status_ = input_->status();
    return byte{0};
  }
  offset_ = 1;
  length_ = len;
  return buffer_[0];
}

inline size_t BufferedMultipassInputStreamIterator::Rep::read(byte* buf,
                                                              size_t count) {
  if (offset_ < length_) {
    // Have some data still in the buffer; just return that.
    if (count > (length_ - offset_)) count = length_ - offset_;
    memcpy(buf, &buffer_[offset_], count);
    offset_ += count;
    return count;
  }
  if (status_ != kOk) {
    // Already done; return the last status.
    return 0;
  }
  if (count >= kMultipassInputStreamIteratorBufferSize) {
    // Skip buffering; read directly into the client's buffer.
    size_t len = input_->read(buf, count);
    if (len == 0) {
      offset_ = 0;
      length_ = 0;
      status_ = input_->status();
    }
    return len;
  }
  size_t len = input_->read(buffer_, kMultipassInputStreamIteratorBufferSize);
  if (len == 0) {
    offset_ = 0;
    length_ = 0;
    status_ = (len == 0 ? kEndOfStream : kReadError);
    return 0;
  }
  length_ = len;
  if (count > length_) count = length_;
  memcpy(buf, buffer_, count);
  offset_ = count;
  return count;
}

inline void BufferedMultipassInputStreamIterator::Rep::skip(size_t count) {
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
