#pragma once

#include "roo_io/stream/multipass_input_stream.h"

namespace roo_io {

class NullInputStream : public MultipassInputStream {
 public:
  NullInputStream(Status error = kClosed) : status_(error) {}

  bool isOpen() const override { return false; }

  void close() override {}

  size_t read(byte* buf, size_t count) override { return 0; }

  void skip(uint64_t count) override {}

  Status status() const override { return status_; }

  uint64_t size() const override { return 0; }

  uint64_t position() const override { return 0; }

  void seek(uint64_t offset) override {}

 private:
  Status status_;
};

}  // namespace roo_io