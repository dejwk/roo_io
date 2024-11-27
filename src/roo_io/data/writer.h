#pragma once

#include "roo_io/data/byte_order.h"
#include "roo_io/iterator/output_iterator.h"

namespace roo_io {

template <typename OutputIterator>
constexpr void WriteBeU16(OutputIterator& out, uint16_t v) {
  out.write((v >> 8) & 0xFF);
  out.write((v >> 0) & 0xFF);
}

template <typename OutputIterator>
constexpr void WriteBeU24(OutputIterator& out, uint32_t v) {
  out.write((v >> 16) & 0xFF);
  out.write((v >> 8) & 0xFF);
  out.write((v >> 0) & 0xFF);
}

template <typename OutputIterator>
constexpr void WriteBeU32(OutputIterator& out, uint32_t v) {
  out.write((v >> 24) & 0xFF);
  out.write((v >> 16) & 0xFF);
  out.write((v >> 8) & 0xFF);
  out.write((v >> 0) & 0xFF);
}

template <typename OutputIterator>
constexpr void WriteBeU64(OutputIterator& out, uint64_t v) {
  WriteBeU32(out, (v >> 32) & 0xFFFFFFFFLL);
  WriteBeU32(out, (v >> 0) & 0xFFFFFFFFLL);
}

template <typename OutputIterator>
constexpr void WriteLeU16(OutputIterator& out, uint16_t v) {
  out.write((v >> 0) & 0xFF);
  out.write((v >> 8) & 0xFF);
}

template <typename OutputIterator>
constexpr void WriteLeU24(OutputIterator& out, uint32_t v) {
  out.write((v >> 0) & 0xFF);
  out.write((v >> 8) & 0xFF);
  out.write((v >> 16) & 0xFF);
}

template <typename OutputIterator>
constexpr void WriteLeU32(OutputIterator& out, uint32_t v) {
  out.write((v >> 0) & 0xFF);
  out.write((v >> 8) & 0xFF);
  out.write((v >> 16) & 0xFF);
  out.write((v >> 24) & 0xFF);
}

template <typename OutputIterator>
constexpr void WriteLeU64(OutputIterator& out, uint64_t v) {
  write_u32(out, (v >> 0) & 0xFFFFFFFFLL);
  write_u32(out, (v >> 32) & 0xFFFFFFFFLL);
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
  uint8_t buffer[10];
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

// Helper to write numbers to output iterators, templated on a byte order.
template <int ByteOrder>
class NumberWriter;

template <>
class NumberWriter<kBigEndian> {
 public:
  template <typename OutputIterator>
  constexpr void writeU16(OutputIterator& out, uint16_t v) const {
    WriteBeU16(out, v);
  }

  template <typename OutputIterator>
  constexpr void writeU24(OutputIterator& out, uint32_t v) const {
    WriteBeU24(out, v);
  }

  template <typename OutputIterator>
  constexpr void writeU32(OutputIterator& out, uint32_t v) const {
    WriteBeU32(out, v);
  }

  template <typename OutputIterator>
  constexpr void writeU64(OutputIterator& out, uint64_t v) const {
    WriteBeU32(out, (v >> 32) & 0xFFFFFFFFLL);
    WriteBeU32(out, (v >> 0) & 0xFFFFFFFFLL);
  }
};

template <>
class NumberWriter<kLittleEndian> {
 public:
  template <typename OutputIterator>
  constexpr void writeU16(OutputIterator& out, uint16_t v) const {
    WriteLeU16(out, v);
  }

  template <typename OutputIterator>
  constexpr void writeU24(OutputIterator& out, uint32_t v) const {
    WriteLeU24(out, v);
  }

  template <typename OutputIterator>
  constexpr void writeU32(OutputIterator& out, uint32_t v) const {
    WriteLeU32(out, v);
  }

  template <typename OutputIterator>
  constexpr void writeU64(OutputIterator& out, uint64_t v) const {
    WriteLeU32(out, (v >> 0) & 0xFFFFFFFFLL);
    WriteLeU32(out, (v >> 32) & 0xFFFFFFFFLL);
  }
};

template <typename OutputIterator, ByteOrder byte_order>
constexpr void WriteU16(OutputIterator& in, uint16_t v) {
  NumberWriter<byte_order>().writeU16(in, v);
}

template <typename OutputIterator, ByteOrder byte_order>
constexpr void WriteU24(OutputIterator& in, uint32_t v) {
  NumberWriter<byte_order>().writeU24(in, v);
}

template <typename OutputIterator, ByteOrder byte_order>
constexpr void WriteU32(OutputIterator& in, uint32_t v) {
  NumberWriter<byte_order>().writeU32(in, v);
}

}  // namespace roo_io
