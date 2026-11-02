#pragma once
#if defined(PICO_RP2040) && PICO_RP2040
#include <hardware/uart.h>
#include <pico/platform.h>

#include "roo_io/core/input_stream.h"
#include "roo_io/core/output_stream.h"
namespace roo_io {
/// Input stream borrowing an initialized UART. Use from one task, outside ISRs.
/// read() waits for at least one byte; tryRead() never waits. close() only
/// closes this adapter. Configure pins, baud and UART framing before
/// construction.
class PicoUartInputStream : public InputStream {
 public:
  /// Borrows @p uart; invalid controllers start with kReadError.
  explicit PicoUartInputStream(uart_inst_t* uart = uart0) : uart_(uart) {
    if (uart != uart0 && uart != uart1) status_ = kReadError;
  }
  /// Copies immediately available bytes; zero-length reads do nothing.
  size_t tryRead(byte* data, size_t count) override {
    if (status_ != kOk || count == 0) return 0;
    if (data == nullptr) {
      status_ = kReadError;
      return 0;
    }
    size_t total = 0;
    while (total < count && uart_is_readable(uart_)) {
      data[total++] = static_cast<byte>(uart_getc(uart_));
    }
    return total;
  }
  /// Blocks until a byte is readable, then returns the available prefix.
  size_t read(byte* data, size_t count) override {
    if (status_ != kOk || count == 0) return 0;
    if (data == nullptr) {
      status_ = kReadError;
      return 0;
    }
    uart_read_blocking(uart_, reinterpret_cast<uint8_t*>(data), 1);
    return 1 + tryRead(data + 1, count - 1);
  }
  /// Reads the requested bytes unless validation or close prevents progress.
  size_t readFully(byte* data, size_t count) override {
    size_t total = 0;
    while (total < count && status_ == kOk) {
      total += read(data == nullptr ? nullptr : data + total, count - total);
    }
    return total;
  }
  /// Releases this view without changing UART hardware.
  void close() override { status_ = kClosed; }
  /// Reports adapter validation/close status; SDK UART has no I/O error result.
  Status status() const override { return status_; }

 private:
  uart_inst_t* uart_;
  Status status_ = kOk;
};

/// Output stream borrowing an initialized UART. Calls from one task, outside
/// ISRs; the application serializes other access to the same UART.
class PicoUartOutputStream : public OutputStream {
 public:
  /// Borrows @p uart; invalid controllers start with kWriteError.
  explicit PicoUartOutputStream(uart_inst_t* uart = uart0) : uart_(uart) {
    if (uart != uart0 && uart != uart1) status_ = kWriteError;
  }
  /// Writes the currently ready prefix without waiting for FIFO space.
  size_t tryWrite(const byte* data, size_t count) override {
    if (status_ != kOk || count == 0) return 0;
    if (data == nullptr) {
      status_ = kWriteError;
      return 0;
    }
    size_t total = 0;
    while (total < count && uart_is_writable(uart_)) {
      uart_putc_raw(uart_, static_cast<char>(data[total++]));
    }
    return total;
  }
  /// Sends the complete buffer, waiting for FIFO space where necessary.
  size_t write(const byte* data, size_t count) override {
    if (status_ != kOk || count == 0) return 0;
    if (data == nullptr) {
      status_ = kWriteError;
      return 0;
    }
    uart_write_blocking(uart_, reinterpret_cast<const uint8_t*>(data), count);
    return count;
  }
  /// Sends the complete buffer with the same contract as write().
  size_t writeFully(const byte* data, size_t count) override {
    return write(data, count);
  }
  /// Waits until the UART has transmitted its queued bytes.
  void flush() override {
    if (status_ == kOk) uart_tx_wait_blocking(uart_);
  }
  /// Flushes then closes this view without deinitializing UART hardware.
  void close() override {
    flush();
    status_ = kClosed;
  }
  /// Reports adapter validation/close status; SDK UART has no I/O error result.
  Status status() const override { return status_; }

 private:
  uart_inst_t* uart_;
  Status status_ = kOk;
};
}  // namespace roo_io
#endif
