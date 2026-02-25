#pragma once

#if (defined ESP_PLATFORM || defined ROO_TESTING)

#include "hal/uart_types.h"
#include "roo_io/core/output_stream.h"

namespace roo_io {

/// `OutputStream` adapter backed by ESP32 UART (esp-idf APIs).
class Esp32UartOutputStream : public OutputStream {
 public:
  /// Creates adapter over ESP32 `uart_port_t`.
  ///
  /// @param uart_num UART port number.
  Esp32UartOutputStream(uart_port_t port) : port_(port), status_(kOk) {}

  /// Non-blocking write.
  ///
  /// Updates status.
  ///
  /// @return Number of bytes written.
  size_t tryWrite(const byte* buf, size_t count) override;

  /// Potentially blocking write.
  ///
  /// Updates status.
  ///
  /// @return Number of bytes written.
  size_t write(const byte* buf, size_t count) override;

  /// Writes exactly `count` bytes unless the UART API call fails.
  ///
  /// Updates status.
  ///
  /// @return Number of bytes written.
  size_t writeFully(const byte* buf, size_t count) override;

  /// Flushes buffered output to the UART sink.
  ///
  /// Updates status.
  void flush() override;

  /// Closes adapter by setting status to `kClosed`.
  ///
  /// Updates status.
  void close() override;

  /// Returns current status.
  ///
  /// @return Current status value.
  Status status() const override { return status_; }

 private:
  size_t available_for_write() const;

  uart_port_t port_;
  mutable Status status_;
};

}  // namespace roo_io

#endif  // (defined ESP_PLATFORM || defined ROO_TESTING)
