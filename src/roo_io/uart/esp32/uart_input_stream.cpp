#if (defined ESP_PLATFORM || defined ROO_TESTING)

#include "roo_io/uart/esp32/uart_input_stream.h"

#include "driver/uart.h"

namespace roo_io {

size_t Esp32UartInputStream::tryRead(roo::byte* buf, size_t count) {
  if (!isOpen() || count == 0) return 0;
  int read = uart_read_bytes(port_, buf, count, 0);
  return read > 0 ? read : 0;
}

size_t Esp32UartInputStream::read(roo::byte* buf, size_t count) {
  if (!isOpen() || count == 0) return 0;
  while (true) {
    int read = uart_read_bytes(port_, buf, count, 0);
    if (read > 0) return read;
    // Block to read at least one byte.
    read = uart_read_bytes(port_, buf, 1, portMAX_DELAY);
    if (read > 0) {
      if (count > static_cast<size_t>(read)) {
        // Opportunistically try to read some more bytes if they're available.
        int more = uart_read_bytes(port_, buf + read,
                                   count - static_cast<size_t>(read), 0);
        if (more < 0) status_ = roo_io::kReadError;
        read += more;
      }
      return read;
    }
    if (read < 0) {
      status_ = roo_io::kReadError;
      return 0;
    }
  }
}

size_t Esp32UartInputStream::readFully(roo::byte* buf, size_t count) {
  if (!isOpen() || count == 0) return 0;
  size_t total = 0;
  while (total < count) {
    int read = uart_read_bytes(port_, buf, count, portMAX_DELAY);
    if (read < 0) {
      status_ = roo_io::kReadError;
      break;
    }
    total += static_cast<size_t>(read);
    buf += read;
    count -= static_cast<size_t>(read);
  }
  return total;
}

}  // namespace roo_io

#endif  // (defined ESP_PLATFORM || defined ROO_TESTING)