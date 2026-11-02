#include "roo_io/stream/arduino/stream_input_stream.h"

#if defined(ARDUINO)
namespace roo_io {
ArduinoStreamInputStream::ArduinoStreamInputStream(Stream& input)
    : input_(input), status_(kOk) {}

bool ArduinoStreamInputStream::isOpen() const { return status_ == kOk; }

size_t ArduinoStreamInputStream::tryRead(byte* buf, size_t count) {
  if (!isOpen() || count == 0) return 0;
  if (buf == nullptr) {
    status_ = kReadError;
    return 0;
  }
  const int available = input_.available();
  if (available <= 0) return 0;
  // Assumes available() accurately reports immediately readable bytes and
  // access to the borrowed Stream is externally serialized through this read.
  // Otherwise readBytes() can wait for the Stream timeout.
  const size_t ready = static_cast<size_t>(available);
  return input_.readBytes(reinterpret_cast<char*>(buf),
                          count < ready ? count : ready);
}

size_t ArduinoStreamInputStream::read(byte* buf, size_t count) {
  if (!isOpen() || count == 0) return 0;
  if (buf == nullptr) {
    status_ = kReadError;
    return 0;
  }
  while (isOpen()) {
    const int available = input_.available();
    const size_t requested =
        available <= 0
            ? 1
            : (count < static_cast<size_t>(available) ? count : available);
    const size_t result =
        input_.readBytes(reinterpret_cast<char*>(buf), requested);
    if (result != 0) return result;
    yield();
  }
  return 0;
}

size_t ArduinoStreamInputStream::readFully(byte* buf, size_t count) {
  size_t total = 0;
  while (total < count && isOpen()) {
    total += read(buf == nullptr ? nullptr : buf + total, count - total);
  }
  return total;
}
}  // namespace roo_io
#endif
