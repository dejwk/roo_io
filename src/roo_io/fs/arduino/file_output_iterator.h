#pragma once

#ifdef ARDUINO

#include <FS.h>

#include <cstring>
#include <memory>

#include "roo_io/core/output_iterator.h"

namespace roo_io {

static const size_t kArduinoFileOutputIteratorBufferSize = 64;

/// Buffered output iterator wrapper around an Arduino `fs::File`.
class ArduinoFileOutputIterator {
 public:
  /// Creates a detached iterator with `kClosed` status.
  ArduinoFileOutputIterator() : rep_(new Rep()) {}

  /// Opens the iterator over an already open Arduino file handle.
  ArduinoFileOutputIterator(::fs::File file) : rep_(new Rep(std::move(file))) {}

  /// Move-constructs the iterator.
  ArduinoFileOutputIterator(ArduinoFileOutputIterator&&) = default;

  /// Flushes buffered data before destruction.
  ~ArduinoFileOutputIterator() { flush(); }

  /// Buffers or writes a single byte.
  void write(byte v) { rep_->write(v); }

  /// Writes up to `count` bytes from `buf`.
  size_t write(const byte* buf, size_t count) {
    return rep_->write(buf, count);
  }

  /// `rep_` can be nullptr after move.
  void flush() {
    if (rep_ != nullptr) rep_->flush();
  }

  /// Returns the current iterator status.
  Status status() const { return rep_->status(); }
  /// Returns whether the iterator status is `kOk`.
  bool ok() const { return status() == roo_io::kOk; }

 private:
  class Rep {
   public:
    Rep();
    Rep(::fs::File file);
    // ~Rep();

    void write(byte v);
    size_t write(const byte* buf, size_t count);
    void flush();

    Status status() const { return status_; }

   private:
    Rep(const Rep&) = delete;
    Rep(Rep&&);
    Rep& operator=(const Rep&);
    void writeBuffer();

    ::fs::File file_;
    byte buffer_[kArduinoFileOutputIteratorBufferSize];
    uint8_t offset_;
    Status status_;
  };

  /// Rep is heap-allocated to keep stack usage low, match underlying heap-
  /// allocated file structures, and keep iterator move operations cheap.
  std::unique_ptr<Rep> rep_;
};

inline ArduinoFileOutputIterator::Rep::Rep()
    : file_(),
      offset_(kArduinoFileOutputIteratorBufferSize),
      status_(kClosed) {}

inline ArduinoFileOutputIterator::Rep::Rep(::fs::File file)
    : file_(std::move(file)), offset_(0), status_(file_ ? kOk : kClosed) {
  if (status_ != kOk) offset_ = kArduinoFileOutputIteratorBufferSize;
}

inline void ArduinoFileOutputIterator::Rep::writeBuffer() {
  if (file_.write((const uint8_t*)buffer_, offset_) < offset_) {
    status_ = kWriteError;
  }
  offset_ = 0;
}

inline void ArduinoFileOutputIterator::Rep::write(byte v) {
  if (offset_ >= kArduinoFileOutputIteratorBufferSize) {
    if (status_ != kOk) return;
    writeBuffer();
  }
  buffer_[offset_++] = v;
}

inline size_t ArduinoFileOutputIterator::Rep::write(const byte* buf,
                                                    size_t len) {
  if (offset_ > 0 || len < kArduinoFileOutputIteratorBufferSize) {
    size_t cap = kArduinoFileOutputIteratorBufferSize - offset_;
    if (len > cap) len = cap;
    memcpy(&buffer_[offset_], buf, len);
    offset_ += len;
    if (offset_ >= kArduinoFileOutputIteratorBufferSize) {
      if (status_ != kOk) return 0;
      writeBuffer();
    }
    return len;
  }
  if (status_ != kOk) return 0;
  size_t result = file_.write((const uint8_t*)buf, len);
  if (result < len) {
    status_ = kWriteError;
  }
  return result;
}

inline void ArduinoFileOutputIterator::Rep::flush() {
  if (status_ == kOk) {
    if (offset_ > 0) writeBuffer();
    file_.flush();
    if (!file_) status_ = kWriteError;
  }
}

}  // namespace roo_io

#endif  // ARDUINO
