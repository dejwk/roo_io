#pragma once

#include <cstring>
#include <memory>

#include "roo_io/iterator/input_iterator.h"
#include "roo_io/stream/multipass_input_stream.h"

namespace roo_io {

static const size_t kMultipassInputStreamIteratorBufferSize = 64;

class BufferedMultipassInputStreamIterator {
 public:
  BufferedMultipassInputStreamIterator() : rep_(new Rep()) {}

  BufferedMultipassInputStreamIterator(roo_io::MultipassInputStream& input)
      : rep_(new Rep(input)) {}

  byte read() { return rep_->read(); }

  int read(byte* buf, unsigned int count) { return rep_->read(buf, count); }

  void skip(unsigned int count) { rep_->skip(count); }
  Status status() const { return rep_->status(); }

  uint64_t size() const { return rep_->size(); }
  uint64_t position() const { return rep_->position(); }

  void rewind() { rep_->rewind(); }
  bool seek(uint64_t position) { return rep_->seek(position); }

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
    int read(byte* buf, unsigned int count);
    void skip(unsigned int count);

    Status status() const { return status_; }
    void reset(roo_io::MultipassInputStream* input);

    uint64_t size() const;
    uint64_t position() const;

    void rewind();
    bool seek(uint64_t position);

   private:
    Rep(const Rep&) = delete;
    Rep(Rep&&);
    Rep& operator=(const Rep&);

    roo_io::MultipassInputStream* input_;
    byte buffer_[kMultipassInputStreamIteratorBufferSize];
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
  uint64_t file_pos = input_->position();
  if (file_pos <= length_) {
    // Keep the buffer data and length.
    offset_ = 0;
  } else {
    // Reset the buffer.
    input_->seek(0);
    offset_ = 0;
    length_ = 0;
    status_ = input_->status();
  }
}

inline bool BufferedMultipassInputStreamIterator::Rep::seek(uint64_t position) {
  uint64_t file_pos = input_->position();
  if (file_pos <= position + length_ && file_pos >= position) {
    // Seek within the area we have in the buffer.
    offset_ = position + length_ - file_pos;
  } else {
    // Seek outside the buffer. Just seek in the file and reset the buffer.
    bool result = input_->seek(position);
    offset_ = 0;
    length_ = 0;
    status_ = input_->status();
    return result;
  }
}

inline byte BufferedMultipassInputStreamIterator::Rep::read() {
  if (offset_ < length_) {
    return buffer_[offset_++];
  }
  if (status_ != kOk) return 0;
  int len = input_->read(buffer_, kMultipassInputStreamIteratorBufferSize);
  if (len <= 0) {
    offset_ = 0;
    length_ = 0;
    status_ = kEndOfStream;
    return 0;
  }
  offset_ = 1;
  length_ = len;
  return buffer_[0];
}

inline int BufferedMultipassInputStreamIterator::Rep::read(byte* buf,
                                                           unsigned int count) {
  if (offset_ < length_) {
    // Have some data still in the buffer; just return that.
    if (count > (length_ - offset_)) count = length_ - offset_;
    memcpy(buf, &buffer_[offset_], count);
    offset_ += count;
    return count;
  }
  if (status_ != kOk) {
    // Already done; return the last status.
    status_ == kEndOfStream ? 0 : status_;
  }
  if (count >= kMultipassInputStreamIteratorBufferSize) {
    // Skip buffering; read directly into the client's buffer.
    int len = input_->read(buf, count);
    if (len <= 0) {
      offset_ = 0;
      length_ = 0;
      status_ = (len == 0 ? kEndOfStream : kReadError);
    }
    return len;
  }
  int len = input_->read(buffer_, kMultipassInputStreamIteratorBufferSize);
  if (len <= 0) {
    offset_ = 0;
    length_ = 0;
    status_ = (len == 0 ? kEndOfStream : kReadError);
    return len;
  }
  length_ = len;
  if (count > length_) count = length_;
  memcpy(buf, buffer_, count);
  offset_ = count;
  return count;
}

inline void BufferedMultipassInputStreamIterator::Rep::skip(
    unsigned int count) {
  unsigned int remaining = (length_ - offset_);
  if (count < remaining) {
    offset_ += count;
  } else {
    offset_ = 0;
    length_ = 0;
    if (status_ != kOk) return;
    if (!input_->skip(count - remaining)) {
      status_ = kEndOfStream;
    }
  }
}

}  // namespace roo_io
