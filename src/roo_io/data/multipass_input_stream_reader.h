#pragma once

#include <memory>

#include "roo_io/core/buffered_multipass_input_stream_iterator.h"
#include "roo_io/core/multipass_input_stream.h"
#include "roo_io/data/byte_order.h"
#include "roo_io/data/ieee754.h"
#include "roo_io/data/read.h"

namespace roo_io {

/// Buffered typed reader over `MultipassInputStream`.
///
/// Uses a 64-byte internal buffer to avoid tiny upstream reads while exposing
/// typed helpers and seek operations.
///
/// Construction with `unique_ptr` transfers ownership.
///
/// After constructing this reader, access the stream only through this reader
/// to keep buffer state coherent.
///
/// Reader closes the stream on destruction or explicit `close()`.
class MultipassInputStreamReader {
 public:
  /// Creates a detached reader with `kClosed` status.
  MultipassInputStreamReader() : in_() {}

  /// Move-constructs the reader and transfers buffered state.
  MultipassInputStreamReader(MultipassInputStreamReader&& other) = default;

  /// Move-assigns the reader and transfers buffered state.
  MultipassInputStreamReader& operator=(MultipassInputStreamReader&& other) =
      default;

  /// Takes ownership of `is` and binds the reader to it when non-null.
  MultipassInputStreamReader(std::unique_ptr<roo_io::MultipassInputStream> is)
      : is_(std::move(is)), in_() {
    if (is_ != nullptr) {
      in_.reset(*is_);
    }
  }

  /// Closes the owned stream, if any.
  ~MultipassInputStreamReader() { close(); }

  /// Replaces the underlying stream and transfers ownership.
  void reset(std::unique_ptr<roo_io::MultipassInputStream> is) {
    if (is_ != nullptr) is_->close();
    is_ = std::move(is);
    if (is_ == nullptr) {
      in_.reset();
    } else {
      in_.reset(*is_);
    }
  }

  /// Closes the current stream and detaches the reader.
  void close() {
    if (is_ == nullptr) return;
    is_->close();
    is_ = nullptr;
    in_.reset();
  }

  /// Returns whether the reader currently references an open stream.
  bool isOpen() const { return in_.ok() || in_.eos(); }

  /// Returns whether the reader status is currently `kOk`.
  bool ok() const { return in_.ok(); }

  /// Returns the size of the underlying stream when open.
  uint64_t size() const { return in_.size(); }
  /// Returns the current read position within the underlying stream.
  uint64_t position() const { return in_.position(); }

  /// Rewinds the reader to the beginning of the underlying stream.
  void rewind() { in_.rewind(); }

  /// Seeks to `position` in the underlying stream.
  void seek(uint64_t position) { in_.seek(position); }

  /// Reads and returns one byte.
  byte read() { return in_.read(); }

  /// Skips up to `count` bytes.
  void skip(size_t count) { in_.skip(count); }

  /// Returns the current iterator status.
  Status status() const { return in_.status(); }

  /// Reads one unsigned byte.
  uint16_t readU8() { return ReadU8(in_); }

  /// Reads one big-endian unsigned 16-bit integer.
  uint16_t readBeU16() { return ReadBeU16(in_); }

  /// Reads one little-endian unsigned 16-bit integer.
  uint16_t readLeU16() { return ReadLeU16(in_); }

  /// Reads one big-endian unsigned 24-bit integer.
  uint32_t readBeU24() { return ReadBeU24(in_); }

  /// Reads one little-endian unsigned 24-bit integer.
  uint32_t readLeU24() { return ReadLeU24(in_); }

  /// Reads one big-endian unsigned 32-bit integer.
  uint32_t readBeU32() { return ReadBeU32(in_); }

  /// Reads one little-endian unsigned 32-bit integer.
  uint32_t readLeU32() { return ReadLeU32(in_); }

  /// Reads one big-endian unsigned 64-bit integer.
  uint64_t readBeU64() { return ReadBeU64(in_); }

  /// Reads one little-endian unsigned 64-bit integer.
  uint64_t readLeU64() { return ReadLeU64(in_); }

  /// Reads one signed byte.
  int16_t readS8() { return ReadS8(in_); }

  /// Reads one big-endian signed 16-bit integer.
  int16_t readBeS16() { return ReadBeS16(in_); }

  /// Reads one little-endian signed 16-bit integer.
  int16_t readLeS16() { return ReadLeS16(in_); }

  /// Reads one big-endian signed 24-bit integer.
  int32_t readBeS24() { return ReadBeS24(in_); }

  /// Reads one little-endian signed 24-bit integer.
  int32_t readLeS24() { return ReadLeS24(in_); }

  /// Reads one big-endian signed 32-bit integer.
  int32_t readBeS32() { return ReadBeS32(in_); }

  /// Reads one little-endian signed 32-bit integer.
  int32_t readLeS32() { return ReadLeS32(in_); }

  /// Reads one big-endian signed 64-bit integer.
  int64_t readBeS64() { return ReadBeS64(in_); }

  /// Reads one little-endian signed 64-bit integer.
  int64_t readLeS64() { return ReadLeS64(in_); }

#if ROO_IO_IEEE754
  /// Reads one big-endian IEEE754 float.
  float readBeFloat() { return ReadBeFloat(in_); }

  /// Reads one little-endian IEEE754 float.
  float readLeFloat() { return ReadLeFloat(in_); }

  /// Reads one big-endian IEEE754 double.
  double readBeDouble() { return ReadBeDouble(in_); }

  /// Reads one little-endian IEEE754 double.
  double readLeDouble() { return ReadLeDouble(in_); }
#endif  // ROO_IO_IEEE754

  /// Reads up to `count` bytes into `result`.
  size_t readByteArray(byte* result, size_t count) {
    return ReadByteArray(in_, result, count);
  }

  /// Reads a length-prefixed string into `buf`, truncating when needed.
  size_t readCString(char* buf, size_t capacity = SIZE_MAX) {
    return ReadCString(in_, buf, capacity);
  }

  /// Reads a length-prefixed string into a `std::string`.
  std::string readString(size_t max_size = SIZE_MAX) {
    return ReadString(in_, max_size);
  }

  /// Reads a host-native trivially copyable value or returns `default_value`.
  template <typename T>
  T readHostNative(T default_value = T()) {
    return HostNativeReader<T>().read(in_, std::move(default_value));
  }

  /// Reads a protobuf-style variable-length unsigned 64-bit integer.
  uint64_t readVarU64() { return ReadVarU64(in_); }

 private:
  std::unique_ptr<roo_io::MultipassInputStream> is_;
  BufferedMultipassInputStreamIterator in_;
};

}  // namespace roo_io
