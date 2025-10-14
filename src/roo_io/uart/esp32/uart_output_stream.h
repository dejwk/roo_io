#pragma once

#if (defined ESP32 || defined ROO_TESTING)

#include "hal/uart_types.h"
#include "roo_io/core/output_stream.h"

namespace roo_io {

// Adepter to write to an arduino stream as an OutputStream. The stream is
// considered opened until close() is explicitly called.
class Esp32UartOutputStream : public OutputStream {
 public:
  Esp32UartOutputStream(uart_port_t port) : port_(port), status_(kOk) {}

  size_t tryWrite(const byte* buf, size_t count) override;

  size_t write(const byte* buf, size_t count) override;

  size_t writeFully(const byte* buf, size_t count) override;

  void flush() override;

  void close() override;

  Status status() const override { return status_; }

 private:
  size_t available_for_write() const;

  uart_port_t port_;
  mutable Status status_;
};

}  // namespace roo_io

#endif  // (defined ESP32 || defined ROO_TESTING)
