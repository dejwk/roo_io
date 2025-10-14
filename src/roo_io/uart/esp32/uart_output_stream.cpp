#if (defined ESP32 || defined ROO_TESTING)

#include "roo_io/uart/esp32/uart_output_stream.h"

#include "driver/uart.h"
#include "hal/uart_ll.h"

namespace roo_io {

size_t Esp32UartOutputStream::tryWrite(const byte* buf, size_t count) {
  if (!isOpen() || count == 0) return 0;
  size_t avail_for_write = available_for_write();
  if (count > avail_for_write) count = avail_for_write;
  if (count == 0) return 0;
  int result = uart_write_bytes(port_, (const char*)buf, count);
  if (result < 0) {
    status_ = roo_io::kWriteError;
    return 0;
  }
  return result;
}

size_t Esp32UartOutputStream::write(const byte* buf, size_t count) {
  if (!isOpen() || count == 0) return 0;
  size_t avail_for_write = available_for_write();
  if (count > avail_for_write) count = avail_for_write;
  if (count == 0) ++count;  // Force at least one byte to be written.
  int result = uart_write_bytes(port_, (const char*)buf, count);
  if (result < 0) {
    status_ = roo_io::kWriteError;
    return 0;
  }
  return result;
}

size_t Esp32UartOutputStream::writeFully(const byte* buf, size_t count) {
  if (!isOpen() || count == 0) return 0;
  int result = uart_write_bytes(port_, (const char*)buf, count);
  if (result < 0) {
    status_ = roo_io::kWriteError;
    return 0;
  }
  return result;
}

void Esp32UartOutputStream::flush() {
  // No-op.
}

void Esp32UartOutputStream::close() {
  if (status_ == kOk) {
    status_ = kClosed;
  }
}

size_t Esp32UartOutputStream::available_for_write() const {
  uint32_t available = uart_ll_get_txfifo_len(UART_LL_GET_HW(port_));
  size_t tx_ringbuf_available = 0;
  if (uart_get_tx_buffer_free_size(port_, &tx_ringbuf_available) == ESP_OK) {
    available = tx_ringbuf_available == 0 ? available : tx_ringbuf_available;
  }
  return available;
}

}  // namespace roo_io

#endif  // ESP32 || ROO_TESTING