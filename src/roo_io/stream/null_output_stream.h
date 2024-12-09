#pragma once

#include "roo_io/stream/output_stream.h"

namespace roo_io {

class NullOutputStream : public OutputStream {
 public:
  NullOutputStream(Status error = kClosed) : status_(error) {}

  void close() override {}

  Status status() const override { return status_; }

  size_t write(const byte* buf, size_t count) override { return 0; }

 private:
  Status status_;
};

}  // namespace roo_io