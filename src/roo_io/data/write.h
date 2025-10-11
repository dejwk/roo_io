#pragma once

#include <string>

#include "roo_backport.h"
#include "roo_backport/string_view.h"
#include "roo_io/core/output_iterator.h"
#include "roo_io/data/byte_order.h"

namespace roo_io {

// Unsigned.

// Writes an unsigned 8-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteU8(OutputIterator& out, uint8_t v) {
  out.write((byte)(v >> 0));
}

// Writes a big-endian unsigned 16-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteBeU16(OutputIterator& out, uint16_t v) {
  out.write((byte)(v >> 8));
  out.write((byte)(v >> 0));
}

// Writes a little-endian unsigned 16-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteLeU16(OutputIterator& out, uint16_t v) {
  out.write((byte)(v >> 0));
  out.write((byte)(v >> 8));
}

// Writes a big-endian unsigned 24-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteBeU24(OutputIterator& out, uint32_t v) {
  out.write((byte)(v >> 16));
  out.write((byte)(v >> 8));
  out.write((byte)(v >> 0));
}

// Writes a little-endian unsigned 24-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteLeU24(OutputIterator& out, uint32_t v) {
  out.write((byte)(v >> 0));
  out.write((byte)(v >> 8));
  out.write((byte)(v >> 16));
}

// Writes a big-endian unsigned 32-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteBeU32(OutputIterator& out, uint32_t v) {
  out.write((byte)(v >> 24));
  out.write((byte)(v >> 16));
  out.write((byte)(v >> 8));
  out.write((byte)(v >> 0));
}

// Writes a little-endian unsigned 32-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteLeU32(OutputIterator& out, uint32_t v) {
  out.write((byte)(v >> 0));
  out.write((byte)(v >> 8));
  out.write((byte)(v >> 16));
  out.write((byte)(v >> 24));
}

// Writes a big-endian unsigned 64-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteBeU64(OutputIterator& out, uint64_t v) {
  WriteBeU32(out, (v >> 32) & 0xFFFFFFFFLL);
  WriteBeU32(out, (v >> 0) & 0xFFFFFFFFLL);
}

// Writes a little-endian unsigned 64-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteLeU64(OutputIterator& out, uint64_t v) {
  WriteLeU32(out, (v >> 0) & 0xFFFFFFFFLL);
  WriteLeU32(out, (v >> 32) & 0xFFFFFFFFLL);
}

// Signed.

// Writes a signed 8-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteS8(OutputIterator& out, int8_t v) {
  WriteU8(out, (uint8_t)v);
}

// Writes a big-endian signed 16-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteBeS16(OutputIterator& out, int16_t v) {
  WriteBeU16(out, (uint16_t)v);
}

// Writes a little-endian signed 16-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteLeS16(OutputIterator& out, int16_t v) {
  WriteLeU16(out, (uint16_t)v);
}

// Writes a big-endian signed 24-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteBeS24(OutputIterator& out, int32_t v) {
  WriteBeU24(out, (uint32_t)v);
}

// Writes a little-endian signed 24-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteLeS24(OutputIterator& out, int32_t v) {
  WriteLeU24(out, (uint32_t)v);
}

// Writes a big-endian signed 32-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteBeS32(OutputIterator& out, int32_t v) {
  WriteBeU32(out, (uint32_t)v);
}

// Writes a little-endian signed 32-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteLeS32(OutputIterator& out, int32_t v) {
  WriteLeU32(out, (uint32_t)v);
}

// Writes a big-endian signed 64-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteBeS64(OutputIterator& out, int64_t v) {
  WriteBeU64(out, (uint64_t)v);
}

// Writes a little-endian signed 64-bit int to the specified iterator.
template <typename OutputIterator>
constexpr void WriteLeS64(OutputIterator& out, int64_t v) {
  WriteLeU64(out, (uint64_t)v);
}

// Writes `count` bytes from the `source` to the output iterator. Returns the
// number of bytes successfully written. If the returned value is smaller than
// `count`, it indicates that an I/O error has occurred. The `status()` of the
// underlying iterator can be used to determine the cause.
template <typename OutputIterator>
size_t WriteByteArray(OutputIterator& out, const byte* source, size_t count) {
  size_t written_total = 0;
  while (count > 0) {
    size_t written_now = out.write(source, count);
    if (written_now == 0) break;
    source += written_now;
    written_total += written_now;
    count -= written_now;
  }
  return written_total;
}

// Writes an unsigned 64-bit integer, encoded using variable-length encoding as
// defined by Google protocol buffers. (Small numbers take little space; numbers
// up to 127 take 1 byte).
template <typename OutputIterator>
void WriteVarU64(OutputIterator& out, uint64_t data) {
  byte buffer[10];
  if (data <= 0x7F) {
    // Fast-path and special-case for single-byte data.
    out.write((byte)data);
    return;
  }

  size_t size = 0;
  while (data > 0) {
    buffer[size++] = ((byte)data & byte{0x7F}) | byte{0x80};
    data >>= 7;
  }
  buffer[size - 1] &= byte{0x7F};
  out.write(buffer, size);
}

// Write a string view, using a portable string representation (varint length
// followed by the character array). Works for std::string.
template <typename OutputIterator>
void WriteString(OutputIterator& itr, roo::string_view data) {
  WriteVarU64(itr, data.size());
  if (!data.empty()) {
    WriteByteArray(itr, (const byte*)data.data(), data.size());
  }
}

// Write a C string, using a portable string representation (varint length
// followed by the character array). Disambiguation of WriteString, since
// both the string view and an Arduino String can be implicitly constructed from
// a C string.
template <typename OutputIterator>
void WriteString(OutputIterator& itr, const char* data) {
  WriteString(itr, roo::string_view(data));
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

// Allows writing platform-native (implementation-dependent) data to an input
// iterator. T must be default-constructible and have trivial destructor.
template <typename T>
struct HostNativeWriter {
 public:
  // Writes T to the iterator. On failure, the iterator status is updated as
  // appropriate.
  template <typename OutputIterator>
  void write(OutputIterator& out, const T& v) const {
    WriteByteArray(out, (const byte*)&v, sizeof(v));
  }
};

}  // namespace roo_io

#if defined(ARDUINO) || defined(String_class_h)

#include "Arduino.h"

// Write an Arduino string, using a portable string representation (varint
// length followed by the character array).
namespace roo_io {
template <typename OutputIterator>
void WriteString(OutputIterator& itr, const ::String& data) {
  WriteVarU64(itr, data.length());
  if (!data.isEmpty()) {
    WriteByteArray(itr, (const byte*)data.c_str(), data.length());
  }
}
}  // namespace roo_io
#endif
