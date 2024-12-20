#pragma once

#include <memory>

#include "roo_io/core/buffered_input_stream_iterator.h"
#include "roo_io/core/input_stream.h"
#include "roo_io/data/byte_order.h"
#include "roo_io/data/read.h"

namespace roo_io {

class InputStreamReader {
 public:
  InputStreamReader() : in_() {}

  InputStreamReader(InputStreamReader&& other) = default;

  InputStreamReader& operator=(InputStreamReader&& other) = default;

  InputStreamReader(std::unique_ptr<roo_io::InputStream> is)
      : is_(std::move(is)), in_(*is_) {}

  ~InputStreamReader() { close(); }

  void reset(std::unique_ptr<roo_io::InputStream> is) {
    if (is_ != nullptr) is_->close();
    is_ = std::move(is);
    if (is_ == nullptr) {
      in_.reset();
    } else {
      in_.reset(*is_);
    }
  }

  void close() {
    if (is_ == nullptr) return;
    is_->close();
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
  std::unique_ptr<roo_io::InputStream> is_;
  BufferedInputStreamIterator in_;
};

}  // namespace roo_io
