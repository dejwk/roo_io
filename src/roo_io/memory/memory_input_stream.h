#pragma once

#include "roo_io/core/multipass_input_stream.h"

namespace roo_io {

template <typename PtrType>
class MemoryInputStream : public MultipassInputStream {
 public:
  MemoryInputStream()
      : ptr_(nullptr), position_(0), size_(0), status_(kClosed) {}

  MemoryInputStream(PtrType begin, PtrType end)
      : ptr_(begin), position_(0), size_(end - begin), status_(kOk) {}

  bool isOpen() const override {
    return status_ == kOk || status_ == kEndOfStream;
  }

  size_t read(byte* buf, size_t count) override {
    if (status_ != kOk) return 0;
    if (position_ >= size_) {
      status_ = kEndOfStream;
      return 0;
    }
    if (position_ + count > size_) {
      count = size_ - position_;
    }
    memcpy(buf, ptr_ + position_, count);
    position_ += count;
    return count;
  }

  void skip(uint64_t count) override {
    if (status_ != kOk) return;
    if (position_ > size_) {
      status_ = kEndOfStream;
      return;
    }
    position_ += count;
    if (position_ > size_) {
      position_ = size_;
      status_ = kEndOfStream;
    }
  }

  uint64_t position() const override { return position_; }

  void seek(uint64_t position) override {
    if (!isOpen()) return;
    position_ = position;
    status_ = kOk;
  }

  uint64_t size() override { return size_; }

  void close() override {
    if (!isOpen()) return;
    position_ = 0;
    status_ = kClosed;
  }

  Status status() const override {
    return status_;
  }

 private:
  PtrType ptr_;
  size_t position_;
  size_t size_;
  Status status_;
};

}  // namespace roo_io