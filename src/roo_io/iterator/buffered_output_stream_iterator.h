#pragma once

#include <cstring>
#include <memory>

#include "roo_io/iterator/output_iterator.h"
#include "roo_io/stream/output_stream.h"

namespace roo_io {

static const size_t kOutputStreamIteratorBufferSize = 64;

class BufferedOutputStreamIterator {
 public:
  BufferedOutputStreamIterator() : rep_(new Rep()) {}

  BufferedOutputStreamIterator(roo_io::OutputStream& output)
      : rep_(new Rep(output)) {}

  void write(uint8_t v) { rep_->write(v); }

  unsigned int write(const uint8_t* buf, unsigned int count) {
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
    // ~Rep();

    void write(uint8_t v);
    unsigned int write(const uint8_t* buf, unsigned int count);
    void flush();

    Status status() const { return status_; }

    void reset(roo_io::OutputStream* output);

   private:
    Rep(const Rep&) = delete;
    Rep(Rep&&);
    Rep& operator=(const Rep&);
    void writeBuffer();

    roo_io::OutputStream* output_;
    uint8_t buffer_[kOutputStreamIteratorBufferSize];
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
    : output_(nullptr), offset_(0), status_(kClosed) {}

inline BufferedOutputStreamIterator::Rep::Rep(roo_io::OutputStream& output)
    : output_(&output), offset_(0), status_(output.status()) {}

inline void BufferedOutputStreamIterator::Rep::reset(
    roo_io::OutputStream* output) {
  output_ = output;
  offset_ = 0;
  status_ = (output != nullptr) ? output->status() : kClosed;
}

inline void BufferedOutputStreamIterator::Rep::writeBuffer() {
  if (status_ != roo_io::kOk) return;
  if (output_->writeFully(buffer_, offset_) < offset_) {
    status_ = output_->status();
  }
  offset_ = 0;
}

inline void BufferedOutputStreamIterator::Rep::write(uint8_t v) {
  buffer_[offset_++] = v;
  if (offset_ >= kOutputStreamIteratorBufferSize) {
    writeBuffer();
  }
}

inline unsigned int BufferedOutputStreamIterator::Rep::write(const uint8_t* buf,
                                                             unsigned int len) {
  if (offset_ > 0 || len < kOutputStreamIteratorBufferSize) {
    int cap = kOutputStreamIteratorBufferSize - offset_;
    if (len > cap) len = cap;
    memcpy(&buffer_[offset_], buf, len);
    offset_ += len;
    if (offset_ >= kOutputStreamIteratorBufferSize) {
      writeBuffer();
    }
    return len;
  }
  if (status_ != roo_io::kOk) return 0;
  int result = output_->writeFully(buf, len);
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
