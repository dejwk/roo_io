#pragma once

#include "roo_io/stream/random_access_input_stream.h"

namespace roo_io {

class NullInputStream : public RandomAccessInputStream {
 public:
  NullInputStream(Status error = kClosed) : status_(error) {}

  bool isOpen() const override { return false; }

  void close() override {}

  int read(uint8_t* buf, unsigned int count) override { return -1; }

  bool skip(uint64_t count) override { return 0; }

  Status status() const override { return status_; }

  uint64_t size() const override { return 0; }

  uint64_t position() const override { return 0; }

  bool seek(uint64_t offset) override { return false; }

 private:
  Status status_;
};

}  // namespace roo_io