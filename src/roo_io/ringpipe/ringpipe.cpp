#include "roo_io/ringpipe/ringpipe.h"

namespace roo_io {

RingPipe::RingPipe(size_t capacity)
    : buffer_(capacity), input_closed_(false), output_closed_(false) {}

size_t RingPipe::write(const byte* data, size_t len) {
  if (len == 0) return 0;
  roo::unique_lock<roo::mutex> lock(mutex_);
  while (buffer_.full() && !output_closed_) {
    not_full_.wait(lock);
  }
  if (input_closed_ || output_closed_) {
    return 0;
  }
  not_empty_.notify_all();
  return buffer_.write(data, len);
}

size_t RingPipe::writeFully(const byte* data, size_t len) {
  size_t written_total = 0;
  while (len > 0) {
    size_t written_now = write(data, len);
    if (written_now == 0) break;
    data += written_now;
    written_total += written_now;
    len -= written_now;
  }
  return written_total;
}

size_t RingPipe::availableForWrite() {
  roo::unique_lock<roo::mutex> lock(mutex_);
  return output_closed_ || input_closed_ ? 0 : buffer_.free();
}

size_t RingPipe::tryWrite(const byte* data, size_t len) {
  if (len == 0) return 0;
  roo::unique_lock<roo::mutex> lock(mutex_);
  if (buffer_.full() || input_closed_ || output_closed_) {
    return 0;
  }
  not_empty_.notify_all();
  return buffer_.write(data, len);
}

size_t RingPipe::read(byte* data, size_t len) {
  if (len == 0) return 0;
  roo::unique_lock<roo::mutex> lock(mutex_);
  while (buffer_.empty()) {
    if (input_closed_ || output_closed_) return 0;
    not_empty_.wait(lock);
  }
  if (input_closed_) {
    return 0;
  }
  not_full_.notify_all();
  return buffer_.read(data, len);
}

size_t RingPipe::availableForRead() {
  roo::unique_lock<roo::mutex> lock(mutex_);
  return buffer_.used();
}

size_t RingPipe::tryRead(byte* data, size_t len) {
  if (len == 0) return 0;
  roo::unique_lock<roo::mutex> lock(mutex_);
  if (buffer_.empty()) {
    return 0;
  }
  not_full_.notify_all();
  return buffer_.read(data, len);
}

Status RingPipe::inputStatus() const {
  roo::unique_lock<roo::mutex> lock(mutex_);
  if (input_closed_) {
    return kClosed;
  }
  if (output_closed_ && buffer_.empty()) {
    return kEndOfStream;
  }
  return kOk;
}

Status RingPipe::outputStatus() const {
  roo::unique_lock<roo::mutex> lock(mutex_);
  if (output_closed_) {
    return kClosed;
  }
  if (input_closed_) {
    return kBrokenPipe;
  }
  return kOk;
}

void RingPipe::closeInput() {
  roo::unique_lock<roo::mutex> lock(mutex_);
  input_closed_ = true;
  buffer_.clear();
  // Awake all readers and writers.
  not_full_.notify_all();
  not_empty_.notify_all();
}

void RingPipe::closeOutput() {
  roo::unique_lock<roo::mutex> lock(mutex_);
  output_closed_ = true;
  // Awake all readers and writers.
  not_full_.notify_all();
  not_empty_.notify_all();
}

}  // namespace roo_io
