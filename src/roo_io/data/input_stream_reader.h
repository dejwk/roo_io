#pragma once

#include <memory>

#include "roo_io/core/buffered_input_stream_iterator.h"
#include "roo_io/core/input_stream.h"
#include "roo_io/data/byte_order.h"
#include "roo_io/data/read.h"

namespace roo_io {

// Convenience wrapper to efficiently read data from the underlying input
// stream. Uses a small buffer (64 bytes) to avoid tiny reads from the
// underlying stream. It owns the stream if constructed with a unique_ptr,
// otherwise it just references it. In any case, once you create a reader, do
// not use the underlying stream explicitly anymore (doing so would interfere
// with the buffer used in this class). The stream is closed when the reader is
// destructed or closed explicitly.
class InputStreamReader {
 public:
  InputStreamReader() : is_(nullptr), owned_(false), in_() {}

  InputStreamReader(InputStreamReader&& other)
      : is_(other.is_), owned_(other.owned_), in_(std::move(other.in_)) {
    other.is_ = nullptr;
    other.owned_ = false;
    other.in_.reset();
  }

  InputStreamReader& operator=(InputStreamReader&& other) {
    if (this != &other) {
      close();
      is_ = other.is_;
      owned_ = other.owned_;
      in_ = std::move(other.in_);
      other.is_ = nullptr;
      other.owned_ = false;
      other.in_.reset();
    }
    return *this;
  }

  InputStreamReader(std::unique_ptr<roo_io::InputStream> is)
      : is_(is.release()), owned_(is_ != nullptr), in_(*is_) {}

  InputStreamReader(roo_io::InputStream& is)
      : is_(&is), owned_(false), in_(*is_) {}

  ~InputStreamReader() {
    if (is_ != nullptr) {
      is_->close();
    }
    if (owned_) delete is_;
  }

  void reset(std::unique_ptr<roo_io::InputStream> is) {
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

  byte read() { return in_.read(); }

  void skip(size_t count) { in_.skip(count); }

  Status status() const { return in_.status(); }

  uint16_t readU8() { return ReadU8(in_); }

  uint16_t readBeU16() { return ReadBeU16(in_); }

  uint16_t readLeU16() { return ReadLeU16(in_); }

  uint32_t readBeU24() { return ReadBeU24(in_); }

  uint32_t readLeU24() { return ReadLeU24(in_); }

  uint32_t readBeU32() { return ReadBeU32(in_); }

  uint32_t readLeU32() { return ReadLeU32(in_); }

  uint64_t readBeU64() { return ReadBeU64(in_); }

  uint64_t readLeU64() { return ReadLeU64(in_); }

  int16_t readS8() { return ReadS8(in_); }

  int16_t readBeS16() { return ReadBeS16(in_); }

  int16_t readLeS16() { return ReadLeS16(in_); }

  int32_t readBeS24() { return ReadBeS24(in_); }

  int32_t readLeS24() { return ReadLeS24(in_); }

  int32_t readBeS32() { return ReadBeS32(in_); }

  int32_t readLeS32() { return ReadLeS32(in_); }

  int64_t readBeS64() { return ReadBeS64(in_); }

  int64_t readLeS64() { return ReadLeS64(in_); }

  size_t readByteArray(byte* result, size_t count) {
    return ReadByteArray(in_, result, count);
  }

  size_t readCString(char* buf, size_t capacity = SIZE_MAX) {
    return ReadCString(in_, buf, capacity);
  }

  std::string readString(size_t max_size = SIZE_MAX) {
    return ReadString(in_, max_size);
  }

  template <typename T>
  T readHostNative(T default_value = T()) {
    return HostNativeReader<T>().read(in_, std::move(default_value));
  }

  uint64_t readVarU64() { return ReadVarU64(in_); }

 private:
  roo_io::InputStream* is_;
  bool owned_;
  BufferedInputStreamIterator in_;
};

}  // namespace roo_io
