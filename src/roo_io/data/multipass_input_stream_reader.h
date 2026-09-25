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
  MultipassInputStreamReader() : in_(), data_error_(false) {}

  /// Move-constructs the reader and transfers buffered state.
  MultipassInputStreamReader(MultipassInputStreamReader&& other)
      : is_(std::move(other.is_)),
        in_(std::move(other.in_)),
        data_error_(other.data_error_) {
    other.data_error_ = false;
  }

  /// Move-assigns the reader and transfers buffered state.
  MultipassInputStreamReader& operator=(MultipassInputStreamReader&& other) {
    if (this != &other) {
      close();
      is_ = std::move(other.is_);
      in_ = std::move(other.in_);
      data_error_ = other.data_error_;
      other.data_error_ = false;
    }
    return *this;
  }

  /// Takes ownership of `is` and binds the reader to it when non-null.
  MultipassInputStreamReader(std::unique_ptr<roo_io::MultipassInputStream> is)
      : is_(std::move(is)), in_(), data_error_(false) {
    if (is_ != nullptr) {
      in_.reset(*is_);
    }
  }

  /// Closes the owned stream, if any.
  ~MultipassInputStreamReader() { close(); }

  /// Replaces the underlying stream and transfers ownership.
  void reset(std::unique_ptr<roo_io::MultipassInputStream> is) {
    data_error_ = false;
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

  /// Returns whether input is healthy and no malformed binary data was read.
  bool ok() const { return status() == kOk && !hasDataError(); }

  /// Returns whether malformed binary data was encountered.
  bool hasDataError() const { return data_error_; }

  /// Returns the size of the underlying stream when open.
  uint64_t size() const { return in_.size(); }
  /// Returns the current read position within the underlying stream.
  uint64_t position() const { return in_.position(); }

  /// Rewinds the reader to the beginning of the underlying stream.
  void rewind() { in_.rewind(); }

  /// Seeks to `position` in the underlying stream.
  void seek(uint64_t position) { in_.seek(position); }

  /// Reads and returns one byte.
  byte read() {
    if (data_error_) return byte{0};
    return in_.read();
  }

  /// Skips up to `count` bytes.
  void skip(size_t count) {
    if (data_error_) return;
    in_.skip(count);
  }

  /// Returns the current iterator status.
  Status status() const { return in_.status(); }

  /// Reads one unsigned byte.
  uint16_t readU8() { return data_error_ ? 0 : ReadU8(in_); }

  /// Reads one big-endian unsigned 16-bit integer.
  uint16_t readBeU16() { return data_error_ ? 0 : ReadBeU16(in_); }

  /// Reads one little-endian unsigned 16-bit integer.
  uint16_t readLeU16() { return data_error_ ? 0 : ReadLeU16(in_); }

  /// Reads one big-endian unsigned 24-bit integer.
  uint32_t readBeU24() { return data_error_ ? 0 : ReadBeU24(in_); }

  /// Reads one little-endian unsigned 24-bit integer.
  uint32_t readLeU24() { return data_error_ ? 0 : ReadLeU24(in_); }

  /// Reads one big-endian unsigned 32-bit integer.
  uint32_t readBeU32() { return data_error_ ? 0 : ReadBeU32(in_); }

  /// Reads one little-endian unsigned 32-bit integer.
  uint32_t readLeU32() { return data_error_ ? 0 : ReadLeU32(in_); }

  /// Reads one big-endian unsigned 64-bit integer.
  uint64_t readBeU64() { return data_error_ ? 0 : ReadBeU64(in_); }

  /// Reads one little-endian unsigned 64-bit integer.
  uint64_t readLeU64() { return data_error_ ? 0 : ReadLeU64(in_); }

  /// Reads one signed byte.
  int16_t readS8() { return data_error_ ? 0 : ReadS8(in_); }

  /// Reads one big-endian signed 16-bit integer.
  int16_t readBeS16() { return data_error_ ? 0 : ReadBeS16(in_); }

  /// Reads one little-endian signed 16-bit integer.
  int16_t readLeS16() { return data_error_ ? 0 : ReadLeS16(in_); }

  /// Reads one big-endian signed 24-bit integer.
  int32_t readBeS24() { return data_error_ ? 0 : ReadBeS24(in_); }

  /// Reads one little-endian signed 24-bit integer.
  int32_t readLeS24() { return data_error_ ? 0 : ReadLeS24(in_); }

  /// Reads one big-endian signed 32-bit integer.
  int32_t readBeS32() { return data_error_ ? 0 : ReadBeS32(in_); }

  /// Reads one little-endian signed 32-bit integer.
  int32_t readLeS32() { return data_error_ ? 0 : ReadLeS32(in_); }

  /// Reads one big-endian signed 64-bit integer.
  int64_t readBeS64() { return data_error_ ? 0 : ReadBeS64(in_); }

  /// Reads one little-endian signed 64-bit integer.
  int64_t readLeS64() { return data_error_ ? 0 : ReadLeS64(in_); }

#if ROO_IO_IEEE754
  /// Reads one big-endian IEEE754 float.
  float readBeFloat() { return data_error_ ? 0 : ReadBeFloat(in_); }

  /// Reads one little-endian IEEE754 float.
  float readLeFloat() { return data_error_ ? 0 : ReadLeFloat(in_); }

  /// Reads one big-endian IEEE754 double.
  double readBeDouble() { return data_error_ ? 0 : ReadBeDouble(in_); }

  /// Reads one little-endian IEEE754 double.
  double readLeDouble() { return data_error_ ? 0 : ReadLeDouble(in_); }
#endif  // ROO_IO_IEEE754

  /// Reads up to `count` bytes into `result`.
  size_t readByteArray(byte* result, size_t count) {
    if (data_error_) return 0;
    return ReadByteArray(in_, result, count);
  }

  /// Reads a strict length-prefixed C string into `buf`.
  ///
  /// `capacity` includes the trailing NUL. Malformed or oversized fields latch
  /// a data error; successful reads return the payload length.
  size_t readCString(char* buf, size_t capacity) {
    if (data_error_) return 0;
    size_t length = 0;
    if (!ReadCString(in_, buf, capacity, &length) && in_.status() == kOk) {
      data_error_ = true;
    }
    return length;
  }

  /// Reads a length-prefixed C string into `buf`, retaining what fits.
  size_t readCStringTruncated(char* buf, size_t capacity) {
    if (data_error_) return 0;
    size_t length = 0;
    if (!ReadCStringTruncated(in_, buf, capacity, &length) &&
        in_.status() == kOk) {
      data_error_ = true;
    }
    return length;
  }

  /// Reads a strict length-prefixed string, limited to `max_size` bytes.
  std::string readString(size_t max_size = SIZE_MAX) {
    if (data_error_) return {};
    std::string result;
    if (!ReadString(in_, &result, max_size) && in_.status() == kOk) {
      data_error_ = true;
    }
    return result;
  }

  /// Reads a length-prefixed string, retaining at most `max_size` bytes.
  std::string readStringTruncated(size_t max_size = SIZE_MAX) {
    if (data_error_) return {};
    std::string result;
    if (!ReadStringTruncated(in_, &result, max_size) && in_.status() == kOk) {
      data_error_ = true;
    }
    return result;
  }

  /// Reads a host-native trivially copyable value or returns `default_value`.
  template <typename T>
  T readHostNative(T default_value = T()) {
    if (data_error_) return default_value;
    return HostNativeReader<T>().read(in_, std::move(default_value));
  }

  /// Reads a checked protobuf-style variable-length unsigned 64-bit integer.
  uint64_t readVarU64() {
    if (data_error_) return 0;
    uint64_t value = 0;
    if (!ReadVarU64(in_, value) && in_.status() == kOk) {
      data_error_ = true;
    }
    return value;
  }

  /// Reads a checked ZigZag-encoded signed 32-bit integer.
  int32_t readZigZag32() {
    if (data_error_) return 0;
    int32_t value = 0;
    if (!ReadZigZag32(in_, value) && in_.status() == kOk) {
      data_error_ = true;
    }
    return value;
  }

  /// Reads a checked ZigZag-encoded signed 64-bit integer.
  int64_t readZigZag64() {
    if (data_error_) return 0;
    int64_t value = 0;
    if (!ReadZigZag64(in_, value) && in_.status() == kOk) {
      data_error_ = true;
    }
    return value;
  }

 private:
  std::unique_ptr<roo_io::MultipassInputStream> is_;
  BufferedMultipassInputStreamIterator in_;
  bool data_error_;
};

}  // namespace roo_io
