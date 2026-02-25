#pragma once

#if (defined ESP_PLATFORM || defined ROO_TESTING)

#include "hal/uart_types.h"
#include "roo_io/core/input_stream.h"

namespace roo_io {

/// `InputStream` adapter backed by ESP32 UART (esp-idf APIs).
///
/// Initialize UART before use. Otherwise reads fail immediately.
class Esp32UartInputStream : public InputStream {
 public:
  /// Creates adapter over ESP32 `uart_port_t`.
  ///
  /// @param uart_num UART port number.
  Esp32UartInputStream(uart_port_t port) : port_(port), status_(kOk) {}

  /// Non-blocking read.
  ///
  /// Updates status.
  ///
  /// @return Number of bytes read.
  size_t tryRead(roo::byte* buf, size_t count) override;

  /// Potentially blocking read.
  ///
  /// Updates status.
  ///
  /// @return Number of bytes read.
  size_t read(roo::byte* buf, size_t count) override;

  /// Reads exactly `count` bytes unless the UART API call fails.
  ///
  /// Updates status.
  ///
  /// @return Number of bytes read.
  size_t readFully(roo::byte* buf, size_t count) override;

  /// Returns whether stream is considered open.
  ///
  /// @return `true` when `status()` is `kOk`.
  bool isOpen() const override { return status_ == roo_io::kOk; }

  /// Closes adapter by setting status to `kClosed`.
  ///
  /// Updates status.
  void close() override { status_ = roo_io::kClosed; }

  /// Returns current status.
  ///
  /// @return Current status value.
  roo_io::Status status() const override { return status_; }

 private:
  uart_port_t port_;
  mutable roo_io::Status status_;
};

}  // namespace roo_io

#endif  // (defined ESP_PLATFORM || defined ROO_TESTING)