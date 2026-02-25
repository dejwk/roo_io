#pragma once

#include <cstring>
#include <memory>

#include "roo_io/core/output_iterator.h"
#include "roo_io/core/output_stream.h"

namespace roo_io {

static const size_t kOutputStreamIteratorBufferSize = 64;

class BufferedOutputStreamIterator {
 public:
  /// Creates a detached iterator with `kClosed` status.
  BufferedOutputStreamIterator()
      : output_(nullptr),
        buffer_(nullptr),
        offset_(kOutputStreamIteratorBufferSize),
        status_(kClosed) {}

  /// Move-constructs iterator state.
  ///
  /// Source iterator becomes detached with `kClosed` status.
  BufferedOutputStreamIterator(BufferedOutputStreamIterator&& other)
      : output_(other.output_),
        buffer_(std::move(other.buffer_)),
        offset_(other.offset_),
        status_(other.status_) {
    other.output_ = nullptr;
    other.offset_ = kOutputStreamIteratorBufferSize;
    other.status_ = kClosed;
  }

  /// Move-assigns iterator state.
  ///
  /// Source iterator becomes detached with `kClosed` status.
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

  /// Creates iterator over `output`.
  ///
  /// Initializes `status()` from `output.status()`. Allocates internal buffer
  /// only when initial status is `kOk`.
  BufferedOutputStreamIterator(roo_io::OutputStream& output)
      : output_(&output), status_(output.status()) {
    if (status_ == kOk) {
      buffer_.reset(new byte[kOutputStreamIteratorBufferSize]);
      offset_ = 0;
    } else {
      offset_ = kOutputStreamIteratorBufferSize;
    }
  }

  /// Flushes pending output on destruction.
  ~BufferedOutputStreamIterator() { flush(); }

  /// Writes one byte.
  ///
  /// If output is healthy, byte is buffered and may trigger flushing buffered
  /// block first. If `status() != kOk` and buffer is full, call is a no-op.
  /// Updates `status()` through internal buffer flush.
  void write(byte v) {
    if (offset_ >= kOutputStreamIteratorBufferSize) {
      if (status_ != kOk) return;
      writeBuffer();
    }
    buffer_[offset_++] = v;
  }

  /// Writes up to `count` bytes.
  ///
  /// May buffer bytes or write directly to underlying stream.
  /// If stream is not writable at entry and no buffer space is available,
  /// returns zero.
  /// Updates `status()` after delegated writes/flushes.
  ///
  /// @return Number of bytes accepted from `buf`.
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

  /// Flushes buffered data and then flushes underlying stream.
  ///
  /// If `status() != kOk`, this call is a no-op.
  /// Updates `status()` from `output.status()`.
  void flush() {
    if (status_ == kOk) {
      if (offset_ > 0) writeBuffer();
      output_->flush();
      status_ = output_->status();
    }
  }

  /// Returns current iterator status.
  ///
  /// @return Current status value.
  Status status() const { return status_; }

  /// Returns whether `status() == kOk`.
  ///
  /// @return `true` iff current status is `kOk`.
  bool ok() const { return status() == roo_io::kOk; }

  /// Detaches from stream and releases internal buffer.
  ///
  /// Sets status to `kClosed`.
  void reset() {
    output_ = nullptr;
    buffer_.reset();
    offset_ = kOutputStreamIteratorBufferSize;
    status_ = kClosed;
  }

  /// Rebinds iterator to `output` and clears buffered state.
  ///
  /// Updates `status()` to `output.status()`. Allocates buffer lazily when
  /// needed and status is `kOk`.
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
