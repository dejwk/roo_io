#include "roo_io/stream/arduino/stream_input_stream.h"

#if (defined ARDUINO)

namespace roo_io {

ArduinoStreamInputStream::ArduinoStreamInputStream(Stream& input)
    : input_(input), status_(kOk) {}

bool ArduinoStreamInputStream::isOpen() const { return status() == kOk; }

size_t ArduinoStreamInputStream::tryRead(byte* buf, size_t count) {
  if (!isOpen() || count == 0) return 0;
  size_t available = input_.available();
  if (count > available) count = available;
  if (count == 0) return 0;
  return input_.readBytes((char*)buf, count);
}

size_t ArduinoStreamInputStream::read(byte* buf, size_t count) {
  if (!isOpen() || count == 0) return 0;
  while (true) {
    size_t available = input_.available();
    if (count > available) count = available;
    // Must read at least one byte.
    if (count == 0) ++count;
    size_t result = input_.readBytes((char*)buf, count);
    if (result > 0) return result;
    yield();
  }
}

size_t ArduinoStreamInputStream::readFully(byte* buf, size_t count) {
  if (!isOpen() || count == 0) return 0;
  size_t total = 0;
  while (true) {
    size_t result = input_.readBytes((char*)buf + total, count - total);
    total += result;
    if (total >= count) return total;
    yield();
  }
}

}  // namespace roo_io

#endif  // (defined ARDUINO)
