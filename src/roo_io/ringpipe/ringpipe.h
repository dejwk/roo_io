#pragma once

#include "roo_io/ringpipe/ringbuffer.h"
#include "roo_io/status.h"
#include "roo_threads.h"
#include "roo_threads/condition_variable.h"
#include "roo_threads/mutex.h"

namespace roo_io {

/// Thread-safe byte pipe backed by a fixed-size in-memory ring buffer.
class RingPipe {
 public:
  /// Constructs a pipe with the specified byte capacity.
  RingPipe(size_t capacity);

  /// Writes at least one and at most `len` bytes, blocking for space if needed.
  ///
  /// Returns zero when either end of the pipe has been closed.
  size_t write(const byte* data, size_t len);

  /// Attempts to write exactly `len` bytes, blocking until progress is
  /// possible.
  ///
  /// Returns fewer than `len` bytes when one end closes before the full write
  /// completes.
  size_t writeFully(const byte* data, size_t len);

  /// Returns the number of bytes that can be written without blocking.
  size_t availableForWrite();

  /// Writes up to `len` bytes without blocking.
  ///
  /// The write is capped by `availableForWrite()` and returns zero when the
  /// pipe is closed or currently full.
  size_t tryWrite(const byte* data, size_t len);

  /// Reads at least one and at most `len` bytes, blocking for data if needed.
  ///
  /// Returns zero when the input end is closed, or when the output end is
  /// closed and all buffered data has been drained.
  size_t read(byte* data, size_t len);

  /// Returns the number of bytes that can be read without blocking.
  size_t availableForRead();

  /// Reads up to `len` bytes without blocking.
  ///
  /// The read is capped by `availableForRead()` and may return zero.
  size_t tryRead(byte* data, size_t len);

  /// Returns the current status visible to readers on the input end.
  Status inputStatus() const;

  /// Returns the current status visible to writers on the output end.
  Status outputStatus() const;

  /// Closes the input end and unblocks writers waiting for additional space.
  void closeInput();

  /// Closes the output end and unblocks readers waiting for additional data.
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
