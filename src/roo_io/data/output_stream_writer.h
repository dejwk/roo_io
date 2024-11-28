#pragma once

#include <memory>

#include "roo_io/data/byte_order.h"
#include "roo_io/data/write.h"
#include "roo_io/iterator/buffered_output_stream_iterator.h"
#include "roo_io/stream/output_stream.h"

namespace roo_io {

class OutputStreamWriter {
 public:
  OutputStreamWriter() : out_() {}

  OutputStreamWriter(OutputStreamWriter&& other) = default;

  OutputStreamWriter(std::unique_ptr<roo_io::OutputStream> os)
      : os_(std::move(os)), out_(*os_) {}

  ~OutputStreamWriter() { close(); }

  void reset(std::unique_ptr<roo_io::OutputStream> os) {
    if (os_ != nullptr) os_->close();
    os_ = std::move(os);
    out_.reset(*os_);
  }

  bool ok() const { return out_.ok(); }

  void flush() {
    if (os_ == nullptr) return;
    out_.flush();
  }

  void close() {
    if (os_ == nullptr) return;
    out_.flush();
    os_->close();
    os_ = nullptr;
    out_.reset();
  }

  Status status() const { return out_.status(); }

  void write(byte v) {
    if (os_ != nullptr) out_.write(v);
  }

  void writeU8(uint8_t v) {
    if (os_ != nullptr) WriteU8(out_, v);
  }

  void writeBeU16(uint16_t v) {
    if (os_ != nullptr) WriteBeU16(out_, v);
  }

  void writeBeU24(uint32_t v) {
    if (os_ != nullptr) WriteBeU24(out_, v);
  }

  void writeBeU32(uint32_t v) {
    if (os_ != nullptr) WriteBeU32(out_, v);
  }

  void writeBeU64(uint64_t v) {
    if (os_ != nullptr) WriteBeU64(out_, v);
  }

  size_t writeByteArray(const byte* source, size_t count) {
    return (os_ == nullptr) ? 0 : WriteByteArray(out_, source, count);
  }

  void writeVarU64(uint64_t data) {
    if (os_ != nullptr) WriteVarU64(out_, data);
  }

 private:
  std::unique_ptr<roo_io::OutputStream> os_;
  BufferedOutputStreamIterator out_;
};

}  // namespace roo_io
