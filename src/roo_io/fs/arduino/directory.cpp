#include "roo_io/fs/arduino/directory.h"

#ifdef ARDUINO

namespace roo_io {

ArduinoDirectoryImpl::ArduinoDirectoryImpl(std::shared_ptr<MountImpl> mount,
                                           fs::File file, Status status)
    : mount_(std::move(mount)), file_(std::move(file)), status_(status) {}

bool ArduinoDirectoryImpl::close() {
  mount_.reset();
  entry_.close();
  file_.close();
  if (status_ == kOk) status_ = kClosed;
  return true;
}

void ArduinoDirectoryImpl::rewind() {
  if (status_ != kOk && status_ != kEndOfStream) return;
  file_.rewindDirectory();
  if (file_) {
    status_ = kOk;
  } else {
    status_ = kUnknownIOError;
    mount_.reset();
  }
  next_ = "";
}

bool ArduinoDirectoryImpl::read(Directory::Entry& entry) {
  if (status_ != kOk) return false;
  if (!file_) {
    status_ = kClosed;
    mount_.reset();
    return false;
  }
  entry_ = file_.openNextFile();
  if (!entry_) {
    status_ = kEndOfStream;
    return false;
  }
#ifdef ARDUINO_ARCH_RP2040
  setEntry(entry, entry_.fullName(),
           strlen(entry_.fullName()) - strlen(entry_.name()),
           entry_.isDirectory());
#else
  setEntry(entry, entry_.path(), strlen(entry_.path()) - strlen(entry_.name()),
           entry_.isDirectory());
#endif
  return true;
}

}  // namespace roo_io

#endif  // ARDUINO
