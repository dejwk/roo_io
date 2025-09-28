#pragma once

#include "roo_io/core/output_stream.h"
#include "roo_io/pipe/pipe.h"

namespace roo_io {

// An OutputStream that writes to a Pipe.
class PipeOutputStream : public OutputStream {
 public:
  explicit PipeOutputStream(Pipe& pipe) : pipe_(pipe) {}

  size_t write(const byte* data, size_t len) override {
    return pipe_.write(data, len);
  }

  size_t tryWrite(const byte* data, size_t len) override {
    return pipe_.tryWrite(data, len);
  }

  size_t availableForWrite() const { return pipe_.availableForWrite(); }

  Status status() const override { return pipe_.outputStatus(); }

  void close() override { pipe_.closeOutput(); }

 private:
  Pipe& pipe_;
};

}  // namespace roo_io