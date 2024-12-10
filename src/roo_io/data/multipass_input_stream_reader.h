#pragma once

#include <memory>

#include "roo_io/data/byte_order.h"
#include "roo_io/data/read.h"
#include "roo_io/iterator/buffered_multipass_input_stream_iterator.h"
#include "roo_io/stream/multipass_input_stream.h"

namespace roo_io {

class MultipassInputStreamReader {
 public:
  MultipassInputStreamReader() : in_() {}

  MultipassInputStreamReader(MultipassInputStreamReader&& other) = default;

  MultipassInputStreamReader(std::unique_ptr<roo_io::MultipassInputStream> is)
      : is_(std::move(is)), in_(*is_) {}

  ~MultipassInputStreamReader() { close(); }

  void reset(std::unique_ptr<roo_io::MultipassInputStream> is) {
    if (is_ != nullptr) is_->close();
    is_ = std::move(is);
    in_.reset(*is_);
  }

  void close() {
    if (is_ == nullptr) return;
    is_->close();
    is_ = nullptr;
    in_.reset();
  }

  bool isOpen() const {
    return is_ != nullptr && is_->isOpen();
  }

  bool ok() const {
    return in_.ok();
  }

  uint64_t size() const { return is_ == nullptr ? 0 : in_.size(); }
  uint64_t position() const { return is_ == nullptr ? 0 : in_.position(); }

  void rewind() {
    if (is_ != nullptr) in_.rewind();
  }

  void seek(uint64_t position) {
    if (is_ != nullptr) in_.seek(position);
  }

  byte read() { return is_ == nullptr ? byte{0} : in_.read(); }

  void skip(size_t count) {
    if (is_ != nullptr) in_.skip(count);
  }

  Status status() const { return in_.status(); }

  uint16_t readBeU16() { return is_ == nullptr ? 0 : ReadBeU16(in_); }

  uint32_t readBeU24() { return is_ == nullptr ? 0 : ReadBeU24(in_); }

  uint32_t readBeU32() { return is_ == nullptr ? 0 : ReadBeU32(in_); }

  size_t readByteArray(byte* result, size_t count) {
    return (is_ == nullptr) ? 0 : ReadByteArray(in_, result, count);
  }

  uint64_t readVarU64() { return is_ == nullptr ? 0 : ReadVarU64(in_); }

 private:
  std::unique_ptr<roo_io::MultipassInputStream> is_;
  BufferedMultipassInputStreamIterator in_;
};

}  // namespace roo_io
