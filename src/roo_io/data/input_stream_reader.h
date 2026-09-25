#pragma once

#include <memory>

#include "roo_io/core/buffered_input_stream_iterator.h"
#include "roo_io/core/input_stream.h"
#include "roo_io/data/byte_order.h"
#include "roo_io/data/ieee754.h"
#include "roo_io/data/read.h"

namespace roo_io {

/// Buffered typed reader over `InputStream`.
///
/// Uses a 64-byte internal buffer to avoid tiny upstream reads.
///
/// Construction with `unique_ptr` transfers ownership; construction with
/// reference does not.
///
/// After constructing this reader, access the stream only through this reader
/// to keep buffer state coherent.
///
/// Reader closes the stream on destruction or explicit `close()`.
class InputStreamReader {
 public:
  InputStreamReader()
      : is_(nullptr), owned_(false), in_(), data_error_(false) {}

  InputStreamReader(InputStreamReader&& other)
      : is_(other.is_),
        owned_(other.owned_),
        in_(std::move(other.in_)),
        data_error_(other.data_error_) {
    other.is_ = nullptr;
    other.owned_ = false;
    other.in_.reset();
    other.data_error_ = false;
  }

  InputStreamReader& operator=(InputStreamReader&& other) {
    if (this != &other) {
      close();
      is_ = other.is_;
      owned_ = other.owned_;
      in_ = std::move(other.in_);
      data_error_ = other.data_error_;
      other.is_ = nullptr;
      other.owned_ = false;
      other.in_.reset();
      other.data_error_ = false;
    }
    return *this;
  }

  InputStreamReader(std::unique_ptr<roo_io::InputStream> is)
      : is_(is.release()), owned_(is_ != nullptr), in_(), data_error_(false) {
    if (is_ != nullptr) {
      in_.reset(*is_);
    }
  }

  InputStreamReader(roo_io::InputStream& is)
      : is_(&is), owned_(false), in_(*is_), data_error_(false) {}

  ~InputStreamReader() {
    if (is_ != nullptr) {
      is_->close();
    }
    if (owned_) delete is_;
  }

  void reset(std::unique_ptr<roo_io::InputStream> is) {
    data_error_ = false;
    if (is_ == is.get()) {
      owned_ = true;
      return;
    }
    if (is_ != nullptr) {
      is_->close();
    }
    if (owned_) {
      delete is_;
    }
    is_ = is.release();
    if (is_ == nullptr) {
      in_.reset();
    } else {
      owned_ = true;
      in_.reset(*is_);
    }
  }

  void reset(roo_io::InputStream& is) {
    data_error_ = false;
    if (is_ == &is) {
      CHECK(!owned_);
      return;
    }
    if (is_ != nullptr) {
      is_->close();
    }
    if (owned_) {
      delete is_;
    }
    is_ = &is;
    owned_ = false;
    in_.reset(*is_);
  }

  void close() {
    if (is_ == nullptr) return;
    is_->close();
    if (owned_) {
      delete is_;
      owned_ = false;
    }
    is_ = nullptr;
    in_.reset();
  }

  byte read() {
    if (data_error_) return byte{0};
    return in_.read();
  }

  void skip(size_t count) {
    if (data_error_) return;
    in_.skip(count);
  }

  Status status() const { return in_.status(); }

  /// Returns whether malformed binary data was encountered.
  bool hasDataError() const { return data_error_; }

  /// Returns whether input is healthy and no malformed binary data was read.
  bool ok() const { return status() == kOk && !hasDataError(); }

  uint16_t readU8() { return data_error_ ? 0 : ReadU8(in_); }

  uint16_t readBeU16() { return data_error_ ? 0 : ReadBeU16(in_); }

  uint16_t readLeU16() { return data_error_ ? 0 : ReadLeU16(in_); }

  uint32_t readBeU24() { return data_error_ ? 0 : ReadBeU24(in_); }

  uint32_t readLeU24() { return data_error_ ? 0 : ReadLeU24(in_); }

  uint32_t readBeU32() { return data_error_ ? 0 : ReadBeU32(in_); }

  uint32_t readLeU32() { return data_error_ ? 0 : ReadLeU32(in_); }

  uint64_t readBeU64() { return data_error_ ? 0 : ReadBeU64(in_); }

  uint64_t readLeU64() { return data_error_ ? 0 : ReadLeU64(in_); }

  int16_t readS8() { return data_error_ ? 0 : ReadS8(in_); }

  int16_t readBeS16() { return data_error_ ? 0 : ReadBeS16(in_); }

  int16_t readLeS16() { return data_error_ ? 0 : ReadLeS16(in_); }

  int32_t readBeS24() { return data_error_ ? 0 : ReadBeS24(in_); }

  int32_t readLeS24() { return data_error_ ? 0 : ReadLeS24(in_); }

  int32_t readBeS32() { return data_error_ ? 0 : ReadBeS32(in_); }

  int32_t readLeS32() { return data_error_ ? 0 : ReadLeS32(in_); }

  int64_t readBeS64() { return data_error_ ? 0 : ReadBeS64(in_); }

  int64_t readLeS64() { return data_error_ ? 0 : ReadLeS64(in_); }

#if ROO_IO_IEEE754
  float readBeFloat() { return data_error_ ? 0 : ReadBeFloat(in_); }

  float readLeFloat() { return data_error_ ? 0 : ReadLeFloat(in_); }

  double readBeDouble() { return data_error_ ? 0 : ReadBeDouble(in_); }

  double readLeDouble() { return data_error_ ? 0 : ReadLeDouble(in_); }
#endif  // ROO_IO_IEEE754

  size_t readByteArray(byte* result, size_t count) {
    if (data_error_) return 0;
    return ReadByteArray(in_, result, count);
  }

  size_t readCString(char* buf, size_t capacity = SIZE_MAX) {
    if (data_error_) return 0;
    return ReadCString(in_, buf, capacity);
  }

  std::string readString(size_t max_size = SIZE_MAX) {
    if (data_error_) return {};
    return ReadString(in_, max_size);
  }

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

 private:
  roo_io::InputStream* is_;
  bool owned_;
  BufferedInputStreamIterator in_;
  bool data_error_;
};

}  // namespace roo_io
