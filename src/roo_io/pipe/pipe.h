#pragma once

#include "roo_io/pipe/ringbuffer.h"
#include "roo_io/status.h"
#include "roo_threads/condition_variable.h"
#include "roo_threads/mutex.h"

namespace roo_io {

// A thread-safe pipe backed by a fixed-size in-memory buffer.
class Pipe {
 public:
  // Constructs a pipe with the given capacity.
  Pipe(size_t capacity);

  // Writes up to 'len' bytes, but at least one byte, to the pipe. Blocks if
  // necessary until space is available. Returns the count of bytes written.
  // If the input or output end has been closed, returns zero.
  size_t write(const byte* data, size_t len);

  // Returns the number of bytes that can be written without blocking. If the
  // input or output end has been closed, returns zero.
  size_t availableForWrite();

  // Writes up to 'len' bytes, but no more than the current value of
  // availableForWrite(), to the pipe. Returns the count of bytes written,
  // which may be zero. If the input or output end has been closed, returns
  // zero.
  size_t tryWrite(const byte* data, size_t len);

  // Reads up to 'len' bytes, but at least one byte, from the pipe. Blocks if
  // necessary until data is available. Returns the count of bytes read. If the
  // output end has been closed and no more data is available, returns zero.
  // If the input end has been closed, returns zero.
  size_t read(byte* data, size_t len);

  // Returns the number of bytes that can be read without blocking. If the
  // input end has been closed, returns zero.
  size_t availableForRead();

  // Reads up to 'len' bytes, but no more than the current value of
  // availableForRead(), from the pipe. Returns the count of bytes read, which
  // may be zero.
  size_t tryRead(byte* data, size_t len);

  // Returns the current status of the input end of the pipe. If the input end
  // has been closed, returns kClosed. If the output end has been closed and
  // no more data is available, returns kEndOfStream. Otherwise, returns kOk.
  Status inputStatus() const;

  // Returns the current status of the output end of the pipe. If the output end
  // has been closed, returns kClosed. If the input end has been closed, returns
  // kBrokenPipe. Otherwise, returns kOk.
  Status outputStatus() const;

  // Closes the input end of the pipe. Further write() or tryWrite() calls, as
  // well as read() or tryRead() calls, will return zero. If there is a
  // write() call blocked waiting for data, it will be unblocked and return
  // zero.
  void closeInput();

  // Closes the output end of the pipe. Further write() or tryWrite() calls will
  // return zero. If there is a read() call blocked waiting for data, it will be
  // unblocked and return zero.
  void closeOutput();

 private:
  RingBuffer buffer_;
  mutable roo::mutex mutex_;
  roo::condition_variable not_empty_;
  roo::condition_variable not_full_;
  bool input_closed_;
  bool output_closed_;
};

}  // namespace roo_io
