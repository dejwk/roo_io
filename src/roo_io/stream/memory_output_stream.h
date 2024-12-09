#pragma once

#include "roo_io/stream/output_stream.h"

namespace roo_io {

template <typename PtrType>
class MemoryOutputStream : public OutputStream {
 public:
  MemoryOutputStream()
      : ptr_(nullptr), end_(nullptr), status_(kClosed) {}

  MemoryOutputStream(PtrType begin, PtrType end)
      : ptr_(begin), end_(end), status_(kOk) {}

  size_t write(const byte* buf, size_t count) override {
    if (status_ != kOk) return 0;
    if (count > end_ - ptr_) {
      count = end_ - ptr_;
      memcpy(ptr_, buf, count);
      status_ = kNoSpaceLeftOnDevice;
      return count;
    }
    memcpy(ptr_, buf, count);
    ptr_ += count;
    return count;
  }

  void close() override {
    if (status_ == kOk) {
      status_ = kClosed;
    }
  }

  Status status() const override { return status_; }

 private:
  byte* ptr_;
  const byte* end_;
  Status status_;
};

}  // namespace roo_io