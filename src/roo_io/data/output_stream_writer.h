#pragma once

#include <memory>

#include "roo_io/core/buffered_output_stream_iterator.h"
#include "roo_io/core/output_stream.h"
#include "roo_io/data/byte_order.h"
#include "roo_io/data/write.h"

namespace roo_io {

// Convenience wrapper to efficiently write typed data to the underlying output
// stream. Uses a small buffer (64 bytes) to avoid tiny writes to the underlying
// stream. It owns the stream if constructed with a unique_ptr, otherwise it
// just references it. In any case, once you create a writer, do not use the
// underlying stream explicitly anymore (doing so would interfere with the
// buffer used in this class). The stream is closed when the writer is
// destructed or closed explicitly.
class OutputStreamWriter {
 public:
  OutputStreamWriter() : os_(nullptr), owned_(false), out_() {}

  OutputStreamWriter(OutputStreamWriter&& other)
      : os_(other.os_), owned_(other.owned_), out_(std::move(other.out_)) {
    other.os_ = nullptr;
    other.owned_ = false;
    other.out_.reset();
  }

  OutputStreamWriter& operator=(OutputStreamWriter&& other) {
    if (this != &other) {
      close();
      os_ = other.os_;
      owned_ = other.owned_;
      out_ = std::move(other.out_);
      other.os_ = nullptr;
      other.owned_ = false;
      other.out_.reset();
    }
    return *this;
  }

  OutputStreamWriter(std::unique_ptr<roo_io::OutputStream> os)
      : os_(os.release()), owned_(os_ != nullptr), out_() {
    if (os_ != nullptr) out_.reset(*os_);
  }

  OutputStreamWriter(roo_io::OutputStream& os)
      : os_(&os), owned_(false), out_(os) {}

  ~OutputStreamWriter() { close(); }

  void reset(std::unique_ptr<roo_io::OutputStream> os) {
    if (os_ == os.get()) {
      owned_ = true;
      return;
    }
    if (os_ != nullptr) {
      os_->close();
      if (owned_) {
        delete os_;
      }
    }
    os_ = os.release();
    owned_ = (os_ != nullptr);
    if (os_ == nullptr) {
      out_.reset();
    } else {
      out_.reset(*os_);
    }
  }

  void reset(roo_io::OutputStream& os) {
    if (os_ == &os) {
      CHECK(!owned_);
      return;
    }
    if (os_ != nullptr) {
      os_->close();
      if (owned_) {
        delete os_;
      }
    }
    os_ = &os;
    owned_ = false;
    out_.reset(*os_);
  }

  bool ok() const { return out_.ok(); }

  void flush() {
    if (os_ == nullptr) return;
    out_.flush();
  }

  void close() {
    if (os_ != nullptr) {
      out_.flush();
      os_->close();
      os_ = nullptr;
      out_.reset();
    }
    if (owned_) {
      delete os_;
      owned_ = false;
    }
  }

  Status status() const { return out_.status(); }

  void write(byte v) { out_.write(v); }

  void writeU8(uint8_t v) { WriteU8(out_, v); }

  void writeBeU16(uint16_t v) { WriteBeU16(out_, v); }

  void writeLeU16(uint16_t v) { WriteLeU16(out_, v); }

  void writeBeU24(uint32_t v) { WriteBeU24(out_, v); }

  void writeLeU24(uint32_t v) { WriteLeU24(out_, v); }

  void writeBeU32(uint32_t v) { WriteBeU32(out_, v); }

  void writeLeU32(uint32_t v) { WriteLeU32(out_, v); }

  void writeBeU64(uint64_t v) { WriteBeU64(out_, v); }

  void writeLeU64(uint64_t v) { WriteLeU64(out_, v); }

  void writeS8(uint8_t v) { WriteS8(out_, v); }

  void writeBeS16(uint16_t v) { WriteBeS16(out_, v); }

  void writeLeS16(uint16_t v) { WriteLeS16(out_, v); }

  void writeBeS24(uint32_t v) { WriteBeS24(out_, v); }

  void writeLeS24(uint32_t v) { WriteLeS24(out_, v); }

  void writeBeS32(uint32_t v) { WriteBeS32(out_, v); }

  void writeLeS32(uint32_t v) { WriteLeS32(out_, v); }

  void writeBeS64(uint64_t v) { WriteBeS64(out_, v); }

  void writeLeS64(uint64_t v) { WriteLeS64(out_, v); }

  size_t writeByteArray(const byte* source, size_t count) {
    return WriteByteArray(out_, source, count);
  }

  void writeString(roo::string_view str) { return WriteString(out_, str); }

  template <typename T>
  void writeHostNative(const T& v) {
    return HostNativeWriter<T>().write(out_, v);
  }

  void writeVarU64(uint64_t data) { return WriteVarU64(out_, data); }

 private:
  roo_io::OutputStream* os_;
  bool owned_;
  BufferedOutputStreamIterator out_;
};

}  // namespace roo_io
