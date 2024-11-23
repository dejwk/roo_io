#pragma once

#include "roo_io/byte_order.h"
#include "roo_io/iterator/output_iterator.h"

namespace roo_io {

// Helper to write numbers from consecutive bytes, assuming a specified
// byte order.
template <int byte_order>
class DataWriter;

template <>
class DataWriter<BIG_ENDIAN> {
 public:
  template <typename OutputIterator>
  constexpr void write_u16(OutputIterator& out, uint16_t v) const {
    out.write((v >> 8) & 0xFF);
    out.write((v >> 0) & 0xFF);
  }

  template <typename OutputIterator>
  constexpr void write_u24(OutputIterator& out, uint32_t v) const {
    out.write((v >> 16) & 0xFF);
    out.write((v >> 8) & 0xFF);
    out.write((v >> 0) & 0xFF);
  }

  template <typename OutputIterator>
  constexpr void write_u32(OutputIterator& out, uint32_t v) const {
    out.write((v >> 24) & 0xFF);
    out.write((v >> 16) & 0xFF);
    out.write((v >> 8) & 0xFF);
    out.write((v >> 0) & 0xFF);
  }

  template <typename OutputIterator>
  constexpr void write_u64(OutputIterator& out, uint64_t v) const {
    write_u32(out, (v >> 32) & 0xFFFFFFFFLL);
    write_u32(out, (v >> 0) & 0xFFFFFFFFLL);
  }

  template <typename OutputIterator>
  constexpr void write_float(OutputIterator& out, float v) const {
    write_u32(out, *reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(&v)));
  }
};

template <>
class DataWriter<LITTLE_ENDIAN> {
 public:
  template <typename OutputIterator>
  constexpr void write_u16(OutputIterator& out, uint16_t v) const {
    out.write((v >> 0) & 0xFF);
    out.write((v >> 8) & 0xFF);
  }

  template <typename OutputIterator>
  constexpr void write_u24(OutputIterator& out, uint32_t v) const {
    out.write((v >> 0) & 0xFF);
    out.write((v >> 8) & 0xFF);
    out.write((v >> 16) & 0xFF);
  }

  template <typename OutputIterator>
  constexpr void write_u32(OutputIterator& out, uint32_t v) const {
    out.write((v >> 0) & 0xFF);
    out.write((v >> 8) & 0xFF);
    out.write((v >> 16) & 0xFF);
    out.write((v >> 24) & 0xFF);
  }

  template <typename OutputIterator>
  constexpr void write_u64(OutputIterator& out, uint64_t v) const {
    write_u32(out, (v >> 0) & 0xFFFFFFFFLL);
    write_u32(out, (v >> 32) & 0xFFFFFFFFLL);
  }

  template <typename OutputIterator>
  constexpr void write_float(OutputIterator& out, float v) const {
    write_u32(out, *reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(&v)));
  }
};

template <typename OutputIterator, int byte_order>
constexpr void WriteU16(OutputIterator& in, uint16_t v) {
  DataWriter<byte_order>().write_u16(in, v);
}

template <typename OutputIterator, int byte_order>
constexpr void WriteU24(OutputIterator& in, uint32_t v) {
  DataWriter<byte_order>().write_u24(in, v);
}

template <typename OutputIterator, int byte_order>
constexpr void WriteU32(OutputIterator& in, uint32_t v) {
  DataWriter<byte_order>().write_u32(in, v);
}

// Convenience helper to write network-encoded (big-endian) uint16.
template <typename OutputIterator>
constexpr void WriteU16be(OutputIterator& in, uint16_t v) {
  DataWriter<BIG_ENDIAN>().write_u16(in, v);
}

// Convenience helper to write network-encoded (big-endian) uint24.
template <typename OutputIterator>
constexpr void WriteU24be(OutputIterator& in, uint32_t v) {
  DataWriter<BIG_ENDIAN>().write_u24(in, v);
}

// Convenience helper to write network-encoded (big-endian) uint32.
template <typename OutputIterator>
constexpr void WriteU32be(OutputIterator& in, uint32_t v) {
  DataWriter<BIG_ENDIAN>().write_u32(in, v);
}

template <typename OutputIterator>
unsigned int WriteByteArray(OutputIterator& out, const uint8_t* source,
                            unsigned int count) {
  unsigned int written_total = 0;
  while (count > 0) {
    int written_now = out.write(source, count);
    if (written_now < 0) break;
    source += written_now;
    written_total += written_now;
    count -= written_now;
  }
  return written_total;
}

template <typename OutputIterator>
void WriteVarU64(OutputIterator& out, uint64_t data) {
  char buffer[10];
  if (data <= 0x7F) {
    // Fast-path and special-case for data == 0.
    out.write(data);
    return;
  }

  size_t size = 0;
  while (data > 0) {
    buffer[size++] = (uint8_t)((data & 0x7F) | 0x80);
    data >>= 7;
  }
  buffer[size - 1] &= 0x7F;
  out.write(buffer, size);
}

}  // namespace roo_io
