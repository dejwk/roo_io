#pragma once

#include <cstring>
#include <limits>
#include <string>

#include "roo_backport.h"
#include "roo_backport/string_view.h"
#include "roo_io/core/output_iterator.h"
#include "roo_io/data/byte_order.h"
#include "roo_io/data/ieee754.h"

namespace roo_io {

// Unsigned.

/// Writes an unsigned 8-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteU8(OutputIterator& out, uint8_t v) {
  out.write((byte)(v >> 0));
}

/// Writes a big-endian unsigned 16-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteBeU16(OutputIterator& out, uint16_t v) {
  out.write((byte)(v >> 8));
  out.write((byte)(v >> 0));
}

/// Writes a little-endian unsigned 16-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteLeU16(OutputIterator& out, uint16_t v) {
  out.write((byte)(v >> 0));
  out.write((byte)(v >> 8));
}

/// Writes a big-endian unsigned 24-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteBeU24(OutputIterator& out, uint32_t v) {
  out.write((byte)(v >> 16));
  out.write((byte)(v >> 8));
  out.write((byte)(v >> 0));
}

/// Writes a little-endian unsigned 24-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteLeU24(OutputIterator& out, uint32_t v) {
  out.write((byte)(v >> 0));
  out.write((byte)(v >> 8));
  out.write((byte)(v >> 16));
}

/// Writes a big-endian unsigned 32-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteBeU32(OutputIterator& out, uint32_t v) {
  out.write((byte)(v >> 24));
  out.write((byte)(v >> 16));
  out.write((byte)(v >> 8));
  out.write((byte)(v >> 0));
}

/// Writes a little-endian unsigned 32-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteLeU32(OutputIterator& out, uint32_t v) {
  out.write((byte)(v >> 0));
  out.write((byte)(v >> 8));
  out.write((byte)(v >> 16));
  out.write((byte)(v >> 24));
}

/// Writes a big-endian unsigned 64-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteBeU64(OutputIterator& out, uint64_t v) {
  WriteBeU32(out, (v >> 32) & 0xFFFFFFFFLL);
  WriteBeU32(out, (v >> 0) & 0xFFFFFFFFLL);
}

/// Writes a little-endian unsigned 64-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteLeU64(OutputIterator& out, uint64_t v) {
  WriteLeU32(out, (v >> 0) & 0xFFFFFFFFLL);
  WriteLeU32(out, (v >> 32) & 0xFFFFFFFFLL);
}

// Signed.

/// Writes a signed 8-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteS8(OutputIterator& out, int8_t v) {
  WriteU8(out, (uint8_t)v);
}

/// Writes a big-endian signed 16-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteBeS16(OutputIterator& out, int16_t v) {
  WriteBeU16(out, (uint16_t)v);
}

/// Writes a little-endian signed 16-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteLeS16(OutputIterator& out, int16_t v) {
  WriteLeU16(out, (uint16_t)v);
}

/// Writes a big-endian signed 24-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteBeS24(OutputIterator& out, int32_t v) {
  WriteBeU24(out, (uint32_t)v);
}

/// Writes a little-endian signed 24-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteLeS24(OutputIterator& out, int32_t v) {
  WriteLeU24(out, (uint32_t)v);
}

/// Writes a big-endian signed 32-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteBeS32(OutputIterator& out, int32_t v) {
  WriteBeU32(out, (uint32_t)v);
}

/// Writes a little-endian signed 32-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteLeS32(OutputIterator& out, int32_t v) {
  WriteLeU32(out, (uint32_t)v);
}

/// Writes a big-endian signed 64-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteBeS64(OutputIterator& out, int64_t v) {
  WriteBeU64(out, (uint64_t)v);
}

/// Writes a little-endian signed 64-bit integer to `out`.
template <typename OutputIterator>
constexpr void WriteLeS64(OutputIterator& out, int64_t v) {
  WriteLeU64(out, (uint64_t)v);
}

#if ROO_IO_IEEE754
/// Writes a big-endian IEEE754 float to `out`.
template <typename OutputIterator>
inline void WriteBeFloat(OutputIterator& out, float v) {
  static_assert(sizeof(float) == sizeof(uint32_t),
                "WriteBeFloat requires 32-bit float.");
  static_assert(std::numeric_limits<float>::is_iec559,
                "WriteBeFloat requires IEEE754 float.");
  uint32_t bits;
  memcpy(&bits, &v, sizeof(bits));
  WriteBeU32(out, bits);
}

/// Writes a little-endian IEEE754 float to `out`.
template <typename OutputIterator>
inline void WriteLeFloat(OutputIterator& out, float v) {
  static_assert(sizeof(float) == sizeof(uint32_t),
                "WriteLeFloat requires 32-bit float.");
  static_assert(std::numeric_limits<float>::is_iec559,
                "WriteLeFloat requires IEEE754 float.");
  uint32_t bits;
  memcpy(&bits, &v, sizeof(bits));
  WriteLeU32(out, bits);
}

/// Writes a big-endian IEEE754 double to `out`.
template <typename OutputIterator>
inline void WriteBeDouble(OutputIterator& out, double v) {
  static_assert(sizeof(double) == sizeof(uint64_t),
                "WriteBeDouble requires 64-bit double.");
  static_assert(std::numeric_limits<double>::is_iec559,
                "WriteBeDouble requires IEEE754 double.");
  uint64_t bits;
  memcpy(&bits, &v, sizeof(bits));
  WriteBeU64(out, bits);
}

/// Writes a little-endian IEEE754 double to `out`.
template <typename OutputIterator>
inline void WriteLeDouble(OutputIterator& out, double v) {
  static_assert(sizeof(double) == sizeof(uint64_t),
                "WriteLeDouble requires 64-bit double.");
  static_assert(std::numeric_limits<double>::is_iec559,
                "WriteLeDouble requires IEEE754 double.");
  uint64_t bits;
  memcpy(&bits, &v, sizeof(bits));
  WriteLeU64(out, bits);
}
#endif  // ROO_IO_IEEE754

/// Writes up to `count` bytes from `source` through `out`.
///
/// Returns the number of bytes transferred. A short write indicates that the
/// iterator entered an error state; inspect `out.status()` for the cause.
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

/// Writes a protobuf-style variable-length unsigned 64-bit integer to `out`.
///
/// This uses the protobuf varint encoding, so values up to 127 occupy one
/// byte.
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

/// Writes `data` using roo_io's portable string encoding.
///
/// The encoding is the varint length followed by the raw character bytes.
template <typename OutputIterator>
void WriteString(OutputIterator& itr, roo::string_view data) {
  WriteVarU64(itr, data.size());
  if (!data.empty()) {
    WriteByteArray(itr, (const byte*)data.data(), data.size());
  }
}

/// Writes a C string using roo_io's portable string encoding.
///
/// The terminating zero is not written. This overload disambiguates calls that
/// would otherwise match both the string-view and Arduino `String` overloads.
template <typename OutputIterator>
void WriteString(OutputIterator& itr, const char* data) {
  WriteString(itr, roo::string_view(data));
}

/// Byte-order-specific integer writer helper.
template <int ByteOrder>
class NumberWriter;

template <>
class NumberWriter<kBigEndian> {
 public:
  /// Writes a big-endian unsigned 16-bit integer.
  template <typename OutputIterator>
  constexpr void writeU16(OutputIterator& out, uint16_t v) const {
    WriteBeU16(out, v);
  }

  /// Writes a big-endian unsigned 24-bit integer.
  template <typename OutputIterator>
  constexpr void writeU24(OutputIterator& out, uint32_t v) const {
    WriteBeU24(out, v);
  }

  /// Writes a big-endian unsigned 32-bit integer.
  template <typename OutputIterator>
  constexpr void writeU32(OutputIterator& out, uint32_t v) const {
    WriteBeU32(out, v);
  }

  /// Writes a big-endian unsigned 64-bit integer.
  template <typename OutputIterator>
  constexpr void writeU64(OutputIterator& out, uint64_t v) const {
    WriteBeU32(out, (v >> 32) & 0xFFFFFFFFLL);
    WriteBeU32(out, (v >> 0) & 0xFFFFFFFFLL);
  }
};

template <>
class NumberWriter<kLittleEndian> {
 public:
  /// Writes a little-endian unsigned 16-bit integer.
  template <typename OutputIterator>
  constexpr void writeU16(OutputIterator& out, uint16_t v) const {
    WriteLeU16(out, v);
  }

  /// Writes a little-endian unsigned 24-bit integer.
  template <typename OutputIterator>
  constexpr void writeU24(OutputIterator& out, uint32_t v) const {
    WriteLeU24(out, v);
  }

  /// Writes a little-endian unsigned 32-bit integer.
  template <typename OutputIterator>
  constexpr void writeU32(OutputIterator& out, uint32_t v) const {
    WriteLeU32(out, v);
  }

  /// Writes a little-endian unsigned 64-bit integer.
  template <typename OutputIterator>
  constexpr void writeU64(OutputIterator& out, uint64_t v) const {
    WriteLeU32(out, (v >> 0) & 0xFFFFFFFFLL);
    WriteLeU32(out, (v >> 32) & 0xFFFFFFFFLL);
  }
};

#if ROO_IO_IEEE754
/// Byte-order-specific IEEE754 floating-point writer helper.
template <int ByteOrder>
class FloatWriter;

template <>
class FloatWriter<kBigEndian> {
 public:
  /// Writes a big-endian IEEE754 float.
  template <typename OutputIterator>
  inline void writeFloat(OutputIterator& out, float v) const {
    WriteBeFloat(out, v);
  }

  /// Writes a big-endian IEEE754 double.
  template <typename OutputIterator>
  inline void writeDouble(OutputIterator& out, double v) const {
    WriteBeDouble(out, v);
  }
};

template <>
class FloatWriter<kLittleEndian> {
 public:
  /// Writes a little-endian IEEE754 float.
  template <typename OutputIterator>
  inline void writeFloat(OutputIterator& out, float v) const {
    WriteLeFloat(out, v);
  }

  /// Writes a little-endian IEEE754 double.
  template <typename OutputIterator>
  inline void writeDouble(OutputIterator& out, double v) const {
    WriteLeDouble(out, v);
  }
};
#endif  // ROO_IO_IEEE754

/// Writes a byte-order-selected unsigned 16-bit integer to `in`.
template <typename OutputIterator, ByteOrder byte_order>
constexpr void WriteU16(OutputIterator& in, uint16_t v) {
  NumberWriter<byte_order>().writeU16(in, v);
}

/// Writes a byte-order-selected unsigned 24-bit integer to `in`.
template <typename OutputIterator, ByteOrder byte_order>
constexpr void WriteU24(OutputIterator& in, uint32_t v) {
  NumberWriter<byte_order>().writeU24(in, v);
}

/// Writes a byte-order-selected unsigned 32-bit integer to `in`.
template <typename OutputIterator, ByteOrder byte_order>
constexpr void WriteU32(OutputIterator& in, uint32_t v) {
  NumberWriter<byte_order>().writeU32(in, v);
}

#if ROO_IO_IEEE754
/// Writes a byte-order-selected IEEE754 float to `in`.
template <typename OutputIterator, ByteOrder byte_order>
inline void WriteFloat(OutputIterator& in, float v) {
  FloatWriter<byte_order>().writeFloat(in, v);
}

/// Writes a byte-order-selected IEEE754 double to `in`.
template <typename OutputIterator, ByteOrder byte_order>
inline void WriteDouble(OutputIterator& in, double v) {
  FloatWriter<byte_order>().writeDouble(in, v);
}
#endif  // ROO_IO_IEEE754

/// Writes host-native trivially copyable values to an output iterator.
template <typename T>
struct HostNativeWriter {
 public:
  /// Writes `v` to `out` using the platform-native representation.
  template <typename OutputIterator>
  void write(OutputIterator& out, const T& v) const {
    WriteByteArray(out, (const byte*)&v, sizeof(v));
  }
};

}  // namespace roo_io

#if defined(ARDUINO) || defined(String_class_h)

#include "Arduino.h"

/// Writes an Arduino `String` using roo_io's portable string encoding.
///
/// The encoding is the varint length followed by the raw character bytes.
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
