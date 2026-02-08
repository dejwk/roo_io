#pragma once

#include <cstring>
#include <memory>

#include "roo_io/core/input_iterator.h"
#include "roo_io/core/input_stream.h"

namespace roo_io {

static const size_t kInputStreamIteratorBufferSize = 64;

class BufferedInputStreamIterator {
 public:
  BufferedInputStreamIterator()
      : input_(nullptr),
        buffer_(nullptr),
        offset_(0),
        length_(0),
        status_(kClosed) {}

  BufferedInputStreamIterator(roo_io::InputStream& input)
      : input_(&input), offset_(0), length_(0), status_(input.status()) {
    buffer_ =
        (status_ == kOk)
            ? std::unique_ptr<byte[]>(new byte[kInputStreamIteratorBufferSize])
            : nullptr;
  }

  BufferedInputStreamIterator(BufferedInputStreamIterator&& other)
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

  BufferedInputStreamIterator& operator=(BufferedInputStreamIterator&& other) {
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
    size_t len = input_->read(buffer_.get(), kInputStreamIteratorBufferSize);
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
      size_t remaining = static_cast<size_t>(length_ - offset_);
      if (count > remaining) count = remaining;
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
    size_t len = input_->read(buffer_.get(), kInputStreamIteratorBufferSize);
    if (len == 0) {
      offset_ = 0;
      length_ = 0;
      status_ = input_->status();
      return 0;
    }
    length_ = len;
    if (count > static_cast<size_t>(length_))
      count = static_cast<size_t>(length_);
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

  bool ok() const { return status() == roo_io::kOk; }

  bool eos() const { return status() == roo_io::kEndOfStream; }

  void reset(roo_io::InputStream& input) {
    input_ = &input;
    offset_ = 0;
    length_ = 0;
    status_ = input.status();
    if (status_ == kOk && buffer_ == nullptr) {
      buffer_ =
          std::unique_ptr<byte[]>(new byte[kInputStreamIteratorBufferSize]);
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
  roo_io::InputStream* input_;
  std::unique_ptr<byte[]> buffer_;
  uint8_t offset_;
  uint8_t length_;
  Status status_;
};

}  // namespace roo_io
