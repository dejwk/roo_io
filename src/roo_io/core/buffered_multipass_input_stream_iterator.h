#pragma once

#include <cstring>
#include <memory>

#include "roo_io/core/input_iterator.h"
#include "roo_io/core/multipass_input_stream.h"

namespace roo_io {

static const size_t kMultipassInputStreamIteratorBufferSize = 64;

class BufferedMultipassInputStreamIterator {
 public:
  /// Creates a detached iterator with `kClosed` status.
  BufferedMultipassInputStreamIterator()
      : input_(nullptr),
        buffer_(nullptr),
        offset_(0),
        length_(0),
        status_(kClosed) {}

  /// Creates iterator over `input`.
  ///
  /// Initializes `status()` from `input.status()`. Allocates internal buffer
  /// when initial status is `kOk` or `kEndOfStream`.
  BufferedMultipassInputStreamIterator(roo_io::MultipassInputStream& input)
      : input_(&input), offset_(0), length_(0), status_(input.status()) {
    buffer_ = (status_ == kOk || status_ == kEndOfStream)
                  ? std::unique_ptr<byte[]>(
                        new byte[kMultipassInputStreamIteratorBufferSize])
                  : nullptr;
  }

  /// Move-constructs iterator state.
  ///
  /// Source iterator becomes detached with `kClosed` status.
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

  /// Move-assigns iterator state.
  ///
  /// Source iterator becomes detached with `kClosed` status.
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

  /// Reads one byte.
  ///
  /// If buffered data is available, returns it without changing `status()`.
  /// If `status() != kOk`, returns zero byte and leaves status unchanged.
  /// Otherwise reads from underlying stream; when that read returns zero,
  /// updates `status()` from `input.status()`.
  ///
  /// @return Read byte, or zero byte when no byte can be read.
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

  /// Reads up to `count` bytes into `buf`.
  ///
  /// Uses buffered bytes first. If `status() != kOk`, returns zero and leaves
  /// status unchanged. When delegated stream read returns zero, updates
  /// `status()` from `input.status()`.
  ///
  /// @return Number of bytes read.
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
    if (count > static_cast<size_t>(length_))
      count = static_cast<size_t>(length_);
    memcpy(buf, buffer_.get(), count);
    offset_ = count;
    return count;
  }

  /// Skips up to `count` bytes.
  ///
  /// If skip is satisfied from buffered bytes, `status()` is unchanged.
  /// Otherwise clears local buffer state and, when `status() == kOk`, delegates
  /// remaining skip to underlying stream and updates `status()` from
  /// `input.status()`.
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

  /// Returns current iterator status.
  ///
  /// @return Current status value.
  Status status() const { return status_; }

  /// Returns stream size when iterator status is `kOk` or `kEndOfStream`.
  ///
  /// Otherwise returns zero.
  ///
  /// @return Stream size, or zero when iterator is in other statuses.
  uint64_t size() const {
    return status_ == kOk || status_ == kEndOfStream ? input_->size() : 0;
  }

  /// Returns current read position when status is `kOk` or `kEndOfStream`.
  ///
  /// Otherwise returns zero.
  ///
  /// @return Current position, or zero when iterator is in other statuses.
  uint64_t position() const {
    return (status_ == kOk || status_ == kEndOfStream)
               ? input_->position() + offset_ - length_
               : 0;
  }

  /// Rewinds to stream start.
  ///
  /// If status is neither `kOk` nor `kEndOfStream`, no-op.
  /// If current position still lies within buffered window, only adjusts
  /// buffer offset and leaves status unchanged.
  /// Otherwise delegates rewind to underlying stream, clears buffer, and
  /// updates `status()` from `input.status()`.
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

  /// Seeks to absolute `position`.
  ///
  /// If status is neither `kOk` nor `kEndOfStream`, no-op.
  /// If target lies within buffered window, adjusts offset only.
  /// Otherwise delegates seek to underlying stream, clears buffer, and
  /// synchronizes from `input.status()`.
  ///
  /// As implemented, accepted seek requests set iterator status to `kOk`
  /// at the end of the call.
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

  /// Returns whether `status() == kOk`.
  ///
  /// @return `true` iff current status is `kOk`.
  bool ok() const { return status() == roo_io::kOk; }

  /// Returns whether `status() == kEndOfStream`.
  ///
  /// @return `true` iff current status is `kEndOfStream`.
  bool eos() const { return status() == roo_io::kEndOfStream; }

  /// Rebinds iterator to `input` and clears buffered state.
  ///
  /// Updates `status()` to `input.status()`. Allocates buffer lazily when
  /// needed and status is `kOk`/`kEndOfStream`.
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

  /// Detaches from stream and releases internal buffer.
  ///
  /// Sets status to `kClosed`.
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
