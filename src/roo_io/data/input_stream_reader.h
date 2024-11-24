#pragma once

#include <memory>

#include "roo_io/data/byte_order.h"
#include "roo_io/data/reader.h"
#include "roo_io/iterator/buffered_input_stream_iterator.h"
#include "roo_io/stream/input_stream.h"

namespace roo_io {

class InputStreamReader {
 public:
  InputStreamReader() : in_() {}

  InputStreamReader(std::unique_ptr<roo_io::InputStream> is)
      : is_(std::move(is)), in_(*is) {}

  void reset(std::unique_ptr<roo_io::InputStream> is) {
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
  std::unique_ptr<roo_io::InputStream> is_;
  BufferedInputStreamIterator in_;
};

}  // namespace roo_io
