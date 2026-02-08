#pragma once

#include <cstring>
#include <memory>

#include "roo_io/core/output_iterator.h"
#include "roo_io/core/output_stream.h"

namespace roo_io {

static const size_t kOutputStreamIteratorBufferSize = 64;

class BufferedOutputStreamIterator {
 public:
  BufferedOutputStreamIterator()
      : output_(nullptr),
        buffer_(nullptr),
        offset_(kOutputStreamIteratorBufferSize),
        status_(kClosed) {}

  BufferedOutputStreamIterator(BufferedOutputStreamIterator&& other)
      : output_(other.output_),
        buffer_(std::move(other.buffer_)),
        offset_(other.offset_),
        status_(other.status_) {
    other.output_ = nullptr;
    other.offset_ = kOutputStreamIteratorBufferSize;
    other.status_ = kClosed;
  }

  BufferedOutputStreamIterator& operator=(
      BufferedOutputStreamIterator&& other) {
    if (this != &other) {
      output_ = other.output_;
      buffer_ = std::move(other.buffer_);
      offset_ = other.offset_;
      status_ = other.status_;
      other.output_ = nullptr;
      other.offset_ = kOutputStreamIteratorBufferSize;
      other.status_ = kClosed;
    }
    return *this;
  }

  BufferedOutputStreamIterator(roo_io::OutputStream& output)
      : output_(&output), status_(output.status()) {
    if (status_ == kOk) {
      buffer_.reset(new byte[kOutputStreamIteratorBufferSize]);
      offset_ = 0;
    } else {
      offset_ = kOutputStreamIteratorBufferSize;
    }
  }

  ~BufferedOutputStreamIterator() { flush(); }

  void write(byte v) {
    if (offset_ >= kOutputStreamIteratorBufferSize) {
      if (status_ != kOk) return;
      writeBuffer();
    }
    buffer_[offset_++] = v;
  }

  size_t write(const byte* buf, size_t count) {
    if (offset_ >= kOutputStreamIteratorBufferSize) {
      if (status_ != kOk) return 0;
      writeBuffer();
    }
    if (offset_ > 0 || count < kOutputStreamIteratorBufferSize) {
      size_t cap = kOutputStreamIteratorBufferSize - offset_;
      if (count > cap) count = cap;
      memcpy(&buffer_[offset_], buf, count);
      offset_ += count;
      return count;
    }
    if (status_ != roo_io::kOk) return 0;
    size_t result = output_->write(buf, count);
    if (result < count) {
      status_ = output_->status();
    }
    return result;
  }

  void flush() {
    if (status_ == kOk) {
      if (offset_ > 0) writeBuffer();
      output_->flush();
      status_ = output_->status();
    }
  }

  Status status() const { return status_; }

  bool ok() const { return status() == roo_io::kOk; }

  void reset() {
    output_ = nullptr;
    buffer_.reset();
    offset_ = kOutputStreamIteratorBufferSize;
    status_ = kClosed;
  }

  void reset(roo_io::OutputStream& output) {
    output_ = &output;
    status_ = output.status();
    if (status_ == kOk) {
      if (buffer_ == nullptr) {
        buffer_.reset(new byte[kOutputStreamIteratorBufferSize]);
      }
      offset_ = 0;
    } else {
      buffer_.reset();
      offset_ = kOutputStreamIteratorBufferSize;
    }
  }

 private:
  inline void writeBuffer() {
    if (output_->writeFully(buffer_.get(), offset_) < offset_) {
      status_ = output_->status();
    }
    offset_ = 0;
  }

  roo_io::OutputStream* output_;
  std::unique_ptr<byte[]> buffer_;
  uint8_t offset_;
  Status status_;
};

}  // namespace roo_io
