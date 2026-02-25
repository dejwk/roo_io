#pragma once

#include <inttypes.h>

#include "roo_io/core/input_stream.h"

namespace roo_io {

/// Virtualizes access to files, memory, and other readable sources.
///
/// Represents an open resource with a seekable read cursor.
///
/// For iterator-style use, prefer `BufferedMultipassInputStreamIterator` to
/// avoid virtual-call overhead per byte.
class MultipassInputStream : public InputStream {
 public:
  /// Returns stream size in bytes from beginning.
  ///
  /// Value may differ across calls when underlying source mutates.
  ///
  /// If pre-call status is neither `kOk` nor `kEndOfStream`, status is
  /// unchanged and return value is 0.
  ///
  /// On error, status updates accordingly and return value is 0.
  virtual uint64_t size() = 0;

  /// Returns current byte offset from beginning of stream.
  ///
  /// If status is neither `kOk` nor `kEndOfStream`, return value is
  /// unspecified.
  virtual uint64_t position() const = 0;

  /// Resets stream to starting position.
  ///
  /// Updates status.
  ///
  /// If pre-call status is neither `kOk` nor `kEndOfStream`, status is
  /// unchanged.
  ///
  /// On success, status becomes `kOk` (clearing EOS if present) and
  /// `position()` becomes 0.
  virtual void rewind() { seek(0); }

  /// Seeks to byte offset from beginning.
  ///
  /// Offset may be greater than current `size()`.
  ///
  /// If pre-call status is neither `kOk` nor `kEndOfStream`, status is
  /// unchanged.
  ///
  /// On success, status becomes `kOk` and `position()` equals `offset`.
  /// On error, status updates accordingly.
  virtual void seek(uint64_t offset) = 0;
};

}  // namespace roo_io
