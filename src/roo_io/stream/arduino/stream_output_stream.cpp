#include "roo_io/stream/arduino/stream_output_stream.h"

#if defined(ARDUINO)
namespace roo_io {
ArduinoStreamOutputStream::ArduinoStreamOutputStream(Stream& output)
    : output_(output), status_(kOk) {}

size_t ArduinoStreamOutputStream::tryWrite(const byte* buf, size_t count) {
  if (status_ != kOk || count == 0) return 0;
  if (buf == nullptr || output_.getWriteError() != 0) {
    status_ = kWriteError;
    return 0;
  }
  const int available = output_.availableForWrite();
  if (available <= 0) return 0;
  if (count > static_cast<size_t>(available)) count = available;
  const size_t written =
      output_.write(reinterpret_cast<const uint8_t*>(buf), count);
  if (written == 0 || output_.getWriteError() != 0) status_ = kWriteError;
  return written;
}

size_t ArduinoStreamOutputStream::write(const byte* buf, size_t count) {
  if (status_ != kOk || count == 0) return 0;
  if (buf == nullptr || output_.getWriteError() != 0) {
    status_ = kWriteError;
    return 0;
  }
  const int available = output_.availableForWrite();
  // A stream without a ready buffer can still perform one blocking write.
  if (available <= 0)
    count = 1;
  else if (count > static_cast<size_t>(available))
    count = available;
  const size_t written =
      output_.write(reinterpret_cast<const uint8_t*>(buf), count);
  if (written == 0 || output_.getWriteError() != 0) status_ = kWriteError;
  return written;
}

size_t ArduinoStreamOutputStream::writeFully(const byte* buf, size_t count) {
  if (status() != kOk) return 0;
  return output_.write((const uint8_t*)buf, count);
}

void ArduinoStreamOutputStream::flush() {
  // Arduino 'flush' is stronger than our semantics, in that it waits for the
  // data to be actually sent out, which we don't require. if (status() != kOk)
  // return; output_.flush();
}

void ArduinoStreamOutputStream::close() {
  flush();
  status_ = kClosed;
}
}  // namespace roo_io
#endif
