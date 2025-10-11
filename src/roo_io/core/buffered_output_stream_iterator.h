#pragma once

#include <cstring>
#include <memory>

#include "roo_io/core/output_iterator.h"
#include "roo_io/core/output_stream.h"

namespace roo_io {

static const size_t kOutputStreamIteratorBufferSize = 64;

class BufferedOutputStreamIterator {
 public:
  BufferedOutputStreamIterator() : rep_(new Rep()) {}

  BufferedOutputStreamIterator(BufferedOutputStreamIterator&& other) = default;

  BufferedOutputStreamIterator(roo_io::OutputStream& output)
      : rep_(new Rep(output)) {}

  ~BufferedOutputStreamIterator() {
    // rep_ gets cleared by std::move(). Generally this is OK, since the object
    // is not supposed to be used after move - but we need to make sure that the
    // destructor works.
    if (rep_ != nullptr) flush();
  }

  void write(byte v) { rep_->write(v); }

  size_t write(const byte* buf, size_t count) {
    return rep_->write(buf, count);
  }

  void flush() { rep_->flush(); }

  Status status() const { return rep_->status(); }
  bool ok() const { return status() == roo_io::kOk; }

  void reset() { rep_->reset(nullptr); }
  void reset(roo_io::OutputStream& output) { rep_->reset(&output); }

 private:
  class Rep {
   public:
    Rep();
    Rep(roo_io::OutputStream& output);
    Rep(Rep&&) = delete;
    // ~Rep();

    void write(byte v);
    size_t write(const byte* buf, size_t count);
    void flush();

    Status status() const { return status_; }

    void reset(roo_io::OutputStream* output);

   private:
    Rep(const Rep&) = delete;
    Rep& operator=(const Rep&);
    void writeBuffer();

    roo_io::OutputStream* output_;
    byte buffer_[kOutputStreamIteratorBufferSize];
    uint8_t offset_;
    Status status_;
  };

  // We keep the content on the heap for the following reasons:
  // * stack space is very limited, and we need some buffer cache;
  // * underlying file structures are using heap anyway;
  // * we want the stream object to be cheaply movable.
  std::unique_ptr<Rep> rep_;
};

inline BufferedOutputStreamIterator::Rep::Rep()
    : output_(nullptr),
      offset_(kOutputStreamIteratorBufferSize),
      status_(kClosed) {}

inline BufferedOutputStreamIterator::Rep::Rep(roo_io::OutputStream& output)
    : output_(&output), offset_(0), status_(output.status()) {
  if (status_ != kOk) offset_ = kOutputStreamIteratorBufferSize;
}

inline void BufferedOutputStreamIterator::Rep::reset(
    roo_io::OutputStream* output) {
  output_ = output;
  offset_ = (output != nullptr) ? 0 : kOutputStreamIteratorBufferSize;
  status_ = (output != nullptr) ? output->status() : kClosed;
}

inline void BufferedOutputStreamIterator::Rep::writeBuffer() {
  if (output_->writeFully(buffer_, offset_) < offset_) {
    status_ = output_->status();
  }
  offset_ = 0;
}

inline void BufferedOutputStreamIterator::Rep::write(byte v) {
  if (offset_ >= kOutputStreamIteratorBufferSize) {
    if (status_ != kOk) return;
    writeBuffer();
  }
  buffer_[offset_++] = v;
}

inline size_t BufferedOutputStreamIterator::Rep::write(const byte* buf,
                                                       size_t len) {
  if (offset_ >= kOutputStreamIteratorBufferSize) {
    if (status_ != kOk) return 0;
    writeBuffer();
  }
  if (offset_ > 0 || len < kOutputStreamIteratorBufferSize) {
    int cap = kOutputStreamIteratorBufferSize - offset_;
    if (len > cap) len = cap;
    memcpy(&buffer_[offset_], buf, len);
    offset_ += len;
    return len;
  }
  if (status_ != roo_io::kOk) return 0;
  size_t result = output_->write(buf, len);
  if (result < len) {
    status_ = output_->status();
  }
  return result;
}

inline void BufferedOutputStreamIterator::Rep::flush() {
  if (status_ == kOk) {
    if (offset_ > 0) writeBuffer();
    output_->flush();
    status_ = output_->status();
  }
}

}  // namespace roo_io
