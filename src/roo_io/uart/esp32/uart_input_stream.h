#pragma once

#if (defined ESP32 || defined ROO_TESTING)

#include "hal/uart_types.h"
#include "roo_io/core/input_stream.h"

namespace roo_io {

// InputStream implementation that reads from an ESP32 UART, using esp-idf
// APIs directly. You need to initialize the UART before using this class,
// otherwise the stream will fail immediately when you try to read from
class Esp32UartInputStream : public InputStream {
 public:
  Esp32UartInputStream(uart_port_t port) : port_(port), status_(kOk) {}

  size_t tryRead(roo::byte* buf, size_t count) override;

  size_t read(roo::byte* buf, size_t count) override;

  size_t readFully(roo::byte* buf, size_t count) override;

  bool isOpen() const override { return status_ == roo_io::kOk; }

  void close() override { status_ = roo_io::kClosed; }

  roo_io::Status status() const override { return status_; }

 private:
  uart_port_t port_;
  mutable roo_io::Status status_;
};

}  // namespace roo_io

#endif  // ESP32 || ROO_TESTING