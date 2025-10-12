#pragma once

#include <cstring>
#include <memory>

#include "roo_io/core/input_iterator.h"
#include "roo_io/core/multipass_input_stream.h"

namespace roo_io {

static const size_t kMultipassInputStreamIteratorBufferSize = 64;

class BufferedMultipassInputStreamIterator {
 public:
  BufferedMultipassInputStreamIterator()
      : input_(nullptr),
        buffer_(nullptr),
        offset_(0),
        length_(0),
        status_(kClosed) {}

  BufferedMultipassInputStreamIterator(roo_io::MultipassInputStream& input)
      : input_(&input), offset_(0), length_(0), status_(input.status()) {
    buffer_ = (status_ == kOk || status_ == kEndOfStream)
                  ? std::unique_ptr<byte[]>(
                        new byte[kMultipassInputStreamIteratorBufferSize])
                  : nullptr;
  }

  BufferedMultipassInputStreamIterator(
      BufferedMultipassInputStreamIterator&& other)
      : input_(other.input_),
        buffer_(std::move(other.buffer_)),
        offset_(other.offset_),
        length_(other.length_),
        status_(other.status_) {
    other.input_ = nullptr;
    other.offset_ = 0;
    other.length_ = 0;
    other.status_ = kClosed;
  }

  BufferedMultipassInputStreamIterator& operator=(
      BufferedMultipassInputStreamIterator&& other) {
    if (this != &other) {
      input_ = other.input_;
      buffer_ = std::move(other.buffer_);
      offset_ = other.offset_;
      length_ = other.length_;
      status_ = other.status_;
      other.input_ = nullptr;
      other.offset_ = 0;
      other.length_ = 0;
      other.status_ = kClosed;
    }
    return *this;
  }

  byte read() {
    if (offset_ < length_) {
      return buffer_[offset_++];
    }
    if (status_ != kOk) return byte{0};
    size_t len =
        input_->read(buffer_.get(), kMultipassInputStreamIteratorBufferSize);
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

  size_t read(byte* buf, size_t count) {
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
    size_t len =
        input_->read(buffer_.get(), kMultipassInputStreamIteratorBufferSize);
    if (len == 0) {
      offset_ = 0;
      length_ = 0;
      status_ = input_->status();
      return 0;
    }
    length_ = len;
    if (count > length_) count = length_;
    memcpy(buf, buffer_.get(), count);
    offset_ = count;
    return count;
  }

  void skip(size_t count) {
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

  Status status() const { return status_; }

  uint64_t size() const {
    return status_ == kOk || status_ == kEndOfStream ? input_->size() : 0;
  }

  uint64_t position() const {
    return (status_ == kOk || status_ == kEndOfStream)
               ? input_->position() + offset_ - length_
               : 0;
  }

  void rewind() {
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

  void seek(uint64_t position) {
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

  bool ok() const { return status() == roo_io::kOk; }

  bool eos() const { return status() == roo_io::kEndOfStream; }

  void reset(roo_io::MultipassInputStream& input) {
    input_ = &input;
    offset_ = 0;
    length_ = 0;
    status_ = input.status();
    if ((status_ == kOk || status_ == kEndOfStream) && buffer_ == nullptr) {
      buffer_ = std::unique_ptr<byte[]>(
          new byte[kMultipassInputStreamIteratorBufferSize]);
    }
  }

  void reset() {
    input_ = nullptr;
    buffer_ = nullptr;
    offset_ = 0;
    length_ = 0;
    status_ = kClosed;
  }

 private:
  roo_io::MultipassInputStream* input_;
  std::unique_ptr<byte[]> buffer_;
  uint8_t offset_;
  uint8_t length_;
  Status status_;
};

}  // namespace roo_io
