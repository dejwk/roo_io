#pragma once

#include "roo_io/core/input_stream.h"
#include "roo_io/pipe/pipe.h"

namespace roo_io {

// An InputStream that reads from a Pipe.
class PipeInputStream : public InputStream {
 public:
  explicit PipeInputStream(Pipe& pipe) : pipe_(pipe) {}

  size_t read(byte* data, size_t len) override { return pipe_.read(data, len); }

  size_t tryRead(byte* data, size_t len) override {
    return pipe_.tryRead(data, len);
  }

  size_t available() const { return pipe_.availableForRead(); }

  Status status() const override { return pipe_.inputStatus(); }

  void close() override { pipe_.closeInput(); }

 private:
  Pipe& pipe_;
};

}  // namespace roo_io
