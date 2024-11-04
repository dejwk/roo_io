#pragma once

#include "roo_io/stream/output_stream.h"

namespace roo_io {

class NullOutputStream : public OutputStream {
 public:
  NullOutputStream(Status error = kClosed) : status_(error) {}

  bool isOpen() const override { return false; }

  void close() override {}

  Status status() const override { return status_; }

  int write(const uint8_t* buf, unsigned int count) override { return -1; }

 private:
  Status status_;
};

}  // namespace roo_io