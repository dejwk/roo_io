#pragma once

#include "roo_io/stream/multipass_input_stream.h"

namespace roo_io {

template <typename PtrType>
class MemoryInputStream : public MultipassInputStream {
 public:
  MemoryInputStream(PtrType begin, PtrType end)
      : begin_(begin), end_(end), current_(begin) {}

  bool isOpen() const override { return current_ != nullptr; }

  int read(byte* buf, unsigned int count) override {
    if (current_ == nullptr) return -1;
    if (count > end_ - current_) {
      count = end_ - current_;
    }
    memcpy(buf, current_, count);
    current_ += count;
    return count;
  }

  bool skip(uint64_t count) override {
    if (count > end_ - current_) {
      current_ = end_;
    } else {
      current_ += count;
    }
    return true;
  }

  // Returns true on success.
  bool seek(uint64_t offset) override {
    if (begin_ + offset > end_) {
      current_ = end_;
    } else {
      current_ = offset;
    }
    return true;
  }

  uint64_t size() const override { return end_ - begin_; }

  bool close() override {
    current_ = nullptr;
    return true;
  }

 private:
  PtrType begin_, end_, current_;
};

}  // namespace roo_io