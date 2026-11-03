#include "roo_io/fs/arduino/file_output_stream.h"

#ifdef ARDUINO

namespace roo_io {

ArduinoFileOutputStream::ArduinoFileOutputStream(Status error)
    : file_(), status_(error) {}

ArduinoFileOutputStream::ArduinoFileOutputStream(fs::File file)
    : ArduinoFileOutputStream(nullptr, std::move(file)) {}

ArduinoFileOutputStream::ArduinoFileOutputStream(
    std::shared_ptr<MountImpl> mount, fs::File file)
    : mount_(std::move(mount)),
      file_(std::move(file)),
      status_(file_ ? kOk : kClosed) {}

size_t ArduinoFileOutputStream::write(const byte* buf, size_t count) {
  if (status_ != kOk || count == 0) return 0;
  size_t result = file_.write((const uint8_t*)buf, count);
  if (result < count) {
    status_ = roo_io::kWriteError;
    file_.close();
    mount_.reset();
  }
  return result;
}

void ArduinoFileOutputStream::flush() {
  if (status_ == kClosed) return;
  file_.flush();
  if (!file_) {
    status_ = kWriteError;
    mount_.reset();
  }
}

void ArduinoFileOutputStream::close() {
  if (status_ == kClosed) return;
  file_.close();
  mount_.reset();
  if (status_ == kOk) {
    status_ = kClosed;
  }
}

}  // namespace roo_io

#endif  // ARDUINO
