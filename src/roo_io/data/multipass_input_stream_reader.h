#pragma once

#include <memory>

#include "roo_io/data/byte_order.h"
#include "roo_io/data/reader.h"
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

  bool seek(uint64_t position) {
    return is_ == nullptr ? false : in_.seek(position);
  }

  uint8_t read() { return is_ == nullptr ? 0 : in_.read(); }

  void skip(unsigned int count) {
    if (is_ != nullptr) in_.skip(count);
  }

  Status status() const { return in_.status(); }

  uint16_t readU16be() { return is_ == nullptr ? 0 : ReadU16be(in_); }

  uint32_t readU24be() { return is_ == nullptr ? 0 : ReadU24be(in_); }

  uint32_t readU32be() { return is_ == nullptr ? 0 : ReadU32be(in_); }

  unsigned int readByteArray(uint8_t* result, unsigned int count) {
    return (is_ == nullptr) ? 0 : ReadByteArray(in_, result, count);
  }

  uint64_t readVarU64() { return is_ == nullptr ? 0 : ReadVarU64(in_); }

 private:
  std::unique_ptr<roo_io::MultipassInputStream> is_;
  BufferedMultipassInputStreamIterator in_;
};

}  // namespace roo_io
