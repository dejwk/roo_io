#pragma once

#include <cstring>
#include <limits>
#include <type_traits>

#include "roo_backport.h"
#include "roo_backport/string_view.h"
#include "roo_io/core/input_iterator.h"
#include "roo_io/data/byte_order.h"
#include "roo_io/data/ieee754.h"
#include "roo_io/memory/memory_input_iterator.h"

namespace roo_io {

// Unsigned.

/// Reads an unsigned 8-bit integer from `in`.
template <typename InputIterator>
constexpr uint8_t ReadU8(InputIterator& in) {
  return (uint8_t)in.read();
}

/// Reads a big-endian unsigned 16-bit integer from `in`.
template <typename InputIterator>
constexpr uint16_t ReadBeU16(InputIterator& in) {
  return ((uint16_t)in.read() << 8) | ((uint16_t)in.read() << 0);
}

/// Reads a little-endian unsigned 16-bit integer from `in`.
template <typename InputIterator>
constexpr uint16_t ReadLeU16(InputIterator& in) {
  return ((uint16_t)in.read() << 0) | ((uint16_t)in.read() << 8);
}

/// Reads a big-endian unsigned 24-bit integer from `in`.
template <typename InputIterator>
constexpr uint32_t ReadBeU24(InputIterator& in) {
  return ((uint32_t)in.read() << 16) | ((uint32_t)in.read() << 8) |
         ((uint32_t)in.read() << 0);
}

/// Reads a little-endian unsigned 24-bit integer from `in`.
template <typename InputIterator>
constexpr uint32_t ReadLeU24(InputIterator& in) {
  return ((uint32_t)in.read() << 0) | ((uint32_t)in.read() << 8) |
         ((uint32_t)in.read() << 16);
}

/// Reads a big-endian unsigned 32-bit integer from `in`.
template <typename InputIterator>
constexpr uint32_t ReadBeU32(InputIterator& in) {
  return ((uint32_t)in.read() << 24) | ((uint32_t)in.read() << 16) |
         ((uint32_t)in.read() << 8) | ((uint32_t)in.read() << 0);
}

/// Reads a little-endian unsigned 32-bit integer from `in`.
template <typename InputIterator>
constexpr uint32_t ReadLeU32(InputIterator& in) {
  return ((uint32_t)in.read() << 0) | ((uint32_t)in.read() << 8) |
         ((uint32_t)in.read() << 16) | ((uint32_t)in.read() << 24);
}

/// Reads a big-endian unsigned 64-bit integer from `in`.
template <typename InputIterator>
constexpr uint64_t ReadBeU64(InputIterator& in) {
  return ((uint64_t)ReadBeU32(in) << 32) | ReadBeU32(in);
}

/// Reads a little-endian unsigned 64-bit integer from `in`.
template <typename InputIterator>
constexpr uint64_t ReadLeU64(InputIterator& in) {
  return ReadLeU32(in) | ((uint64_t)ReadLeU32(in) << 32);
}

// Signed.

/// Reads a signed 8-bit integer from `in`.
template <typename InputIterator>
constexpr int8_t ReadS8(InputIterator& in) {
  return (int8_t)in.read();
}

/// Reads a big-endian signed 16-bit integer from `in`.
template <typename InputIterator>
constexpr int16_t ReadBeS16(InputIterator& in) {
  return (int16_t)ReadBeU16(in);
}

/// Reads a little-endian signed 16-bit integer from `in`.
template <typename InputIterator>
constexpr int16_t ReadLeS16(InputIterator& in) {
  return (int16_t)ReadLeU16(in);
}

namespace internal {

// Copies the sign bit (23th bit) to the top 8 bits.
constexpr int32_t sign_extend_24(int32_t v) {
  return v | (((v & 0x00800000) > 0) * 0xFF000000);
}

}  // namespace internal

/// Reads a big-endian signed 24-bit integer from `in`.
template <typename InputIterator>
constexpr int32_t ReadBeS24(InputIterator& in) {
  return internal::sign_extend_24((int32_t)ReadBeU24(in));
}

/// Reads a little-endian signed 24-bit integer from `in`.
template <typename InputIterator>
constexpr int32_t ReadLeS24(InputIterator& in) {
  return internal::sign_extend_24((int32_t)ReadLeU24(in));
}

/// Reads a big-endian signed 32-bit integer from `in`.
template <typename InputIterator>
constexpr int32_t ReadBeS32(InputIterator& in) {
  return (int32_t)ReadBeU32(in);
}

/// Reads a little-endian signed 32-bit integer from `in`.
template <typename InputIterator>
constexpr int32_t ReadLeS32(InputIterator& in) {
  return (int32_t)ReadLeU32(in);
}

/// Reads a big-endian signed 64-bit integer from `in`.
template <typename InputIterator>
constexpr int64_t ReadBeS64(InputIterator& in) {
  return (int64_t)ReadBeU64(in);
}

/// Reads a little-endian signed 64-bit integer from `in`.
template <typename InputIterator>
constexpr int64_t ReadLeS64(InputIterator& in) {
  return (int64_t)ReadLeU64(in);
}

#if ROO_IO_IEEE754
/// Reads a big-endian IEEE754 float from `in`.
template <typename InputIterator>
inline float ReadBeFloat(InputIterator& in) {
  static_assert(sizeof(float) == sizeof(uint32_t),
                "ReadBeFloat requires 32-bit float.");
  static_assert(std::numeric_limits<float>::is_iec559,
                "ReadBeFloat requires IEEE754 float.");
  uint32_t bits = ReadBeU32(in);
  float value;
  memcpy(&value, &bits, sizeof(value));
  return value;
}

/// Reads a little-endian IEEE754 float from `in`.
template <typename InputIterator>
inline float ReadLeFloat(InputIterator& in) {
  static_assert(sizeof(float) == sizeof(uint32_t),
                "ReadLeFloat requires 32-bit float.");
  static_assert(std::numeric_limits<float>::is_iec559,
                "ReadLeFloat requires IEEE754 float.");
  uint32_t bits = ReadLeU32(in);
  float value;
  memcpy(&value, &bits, sizeof(value));
  return value;
}

/// Reads a big-endian IEEE754 double from `in`.
template <typename InputIterator>
inline double ReadBeDouble(InputIterator& in) {
  static_assert(sizeof(double) == sizeof(uint64_t),
                "ReadBeDouble requires 64-bit double.");
  static_assert(std::numeric_limits<double>::is_iec559,
                "ReadBeDouble requires IEEE754 double.");
  uint64_t bits = ReadBeU64(in);
  double value;
  memcpy(&value, &bits, sizeof(value));
  return value;
}

/// Reads a little-endian IEEE754 double from `in`.
template <typename InputIterator>
inline double ReadLeDouble(InputIterator& in) {
  static_assert(sizeof(double) == sizeof(uint64_t),
                "ReadLeDouble requires 64-bit double.");
  static_assert(std::numeric_limits<double>::is_iec559,
                "ReadLeDouble requires IEEE754 double.");
  uint64_t bits = ReadLeU64(in);
  double value;
  memcpy(&value, &bits, sizeof(value));
  return value;
}
#endif  // ROO_IO_IEEE754

/// Reads up to `count` bytes from `in` into `result`.
///
/// Returns the number of bytes transferred. A short read means that the end of
/// stream was reached or that the iterator entered an error state; inspect
/// `in.status()` to distinguish the two.
template <typename InputIterator>
size_t ReadByteArray(InputIterator& in, byte* result, size_t count) {
  size_t read_total = 0;
  while (count > 0) {
    size_t read_now = in.read(result, count);
    if (read_now == 0) break;
    result += read_now;
    read_total += read_now;
    count -= read_now;
  }
  return read_total;
}

/// Reads a protobuf-style variable-length unsigned 64-bit integer from `in`.
///
/// This uses the protobuf varint encoding, so values up to 127 occupy one
/// byte. Returns zero if the iterator leaves the `kOk` state while decoding.
template <typename InputIterator>
uint64_t ReadVarU64(InputIterator& in) {
  uint64_t result = 0;
  byte read;
  int shift = 0;
  do {
    read = in.read();
    if (in.status() != kOk) {
      return 0;
    }
    result |= ((uint64_t)(read & byte{0x7F}) << shift);
    shift += 7;
  } while ((read & byte{0x80}) != byte{0});
  return result;
}

/// Byte-order-specific integer reader helper.
template <ByteOrder byte_order>
class IntegerReader;

template <>
class IntegerReader<kBigEndian> {
 public:
  /// Reads a big-endian unsigned 16-bit integer.
  template <typename InputIterator>
  constexpr uint16_t readU16(InputIterator& in) const {
    return ReadBeU16(in);
  }

  /// Reads a big-endian unsigned 24-bit integer.
  template <typename InputIterator>
  constexpr uint32_t readU24(InputIterator& in) const {
    return ReadBeU24(in);
  }

  /// Reads a big-endian unsigned 32-bit integer.
  template <typename InputIterator>
  constexpr uint32_t readU32(InputIterator& in) const {
    return ReadBeU32(in);
  }

  /// Reads a big-endian unsigned 64-bit integer.
  template <typename InputIterator>
  constexpr uint64_t readU64(InputIterator& in) const {
    return ReadBeU64(in);
  }

  /// Reads a big-endian signed 16-bit integer.
  template <typename InputIterator>
  constexpr int16_t readS16(InputIterator& in) const {
    return ReadBeS16(in);
  }

  /// Reads a big-endian signed 24-bit integer.
  template <typename InputIterator>
  constexpr int32_t readS24(InputIterator& in) const {
    return ReadBeS24(in);
  }

  /// Reads a big-endian signed 32-bit integer.
  template <typename InputIterator>
  constexpr int32_t readS32(InputIterator& in) const {
    return ReadBeS32(in);
  }

  /// Reads a big-endian signed 64-bit integer.
  template <typename InputIterator>
  constexpr int64_t readS64(InputIterator& in) const {
    return ReadBeS64(in);
  }

  // template <typename InputIterator>
  // constexpr float read_float(InputIterator& in) const {
  //   return *reinterpret_cast<float*>(reinterpret_cast<char*>(&read_u32(in)));
  // }
};

template <>
class IntegerReader<kLittleEndian> {
 public:
  /// Reads a little-endian unsigned 16-bit integer.
  template <typename InputIterator>
  constexpr uint16_t readU16(InputIterator& in) const {
    return ReadLeU16(in);
  }

  /// Reads a little-endian unsigned 24-bit integer.
  template <typename InputIterator>
  constexpr uint32_t readU24(InputIterator& in) const {
    return ReadLeU24(in);
  }

  /// Reads a little-endian unsigned 32-bit integer.
  template <typename InputIterator>
  constexpr uint32_t readU32(InputIterator& in) const {
    return ReadLeU32(in);
  }

  /// Reads a little-endian unsigned 64-bit integer.
  template <typename InputIterator>
  constexpr uint64_t readU64(InputIterator& in) const {
    return ReadLeU64(in);
  }

  /// Reads a little-endian signed 16-bit integer.
  template <typename InputIterator>
  constexpr int16_t readS16(InputIterator& in) const {
    return ReadLeS16(in);
  }

  /// Reads a little-endian signed 24-bit integer.
  template <typename InputIterator>
  constexpr int32_t readS24(InputIterator& in) const {
    return ReadLeS24(in);
  }

  /// Reads a little-endian signed 32-bit integer.
  template <typename InputIterator>
  constexpr int32_t readS32(InputIterator& in) const {
    return ReadLeS32(in);
  }

  /// Reads a little-endian signed 64-bit integer.
  template <typename InputIterator>
  constexpr int64_t readS64(InputIterator& in) const {
    return ReadLeS64(in);
  }

  // template <typename InputIterator>
  // constexpr float read_float(InputIterator& in) const {
  //   return *reinterpret_cast<float*>(reinterpret_cast<char*>(&read_u32(in)));
  // }
};

#if ROO_IO_IEEE754
/// Byte-order-specific IEEE754 floating-point reader helper.
template <ByteOrder byte_order>
class FloatReader;

template <>
class FloatReader<kBigEndian> {
 public:
  /// Reads a big-endian IEEE754 float.
  template <typename InputIterator>
  inline float readFloat(InputIterator& in) const {
    return ReadBeFloat(in);
  }

  /// Reads a big-endian IEEE754 double.
  template <typename InputIterator>
  inline double readDouble(InputIterator& in) const {
    return ReadBeDouble(in);
  }
};

template <>
class FloatReader<kLittleEndian> {
 public:
  /// Reads a little-endian IEEE754 float.
  template <typename InputIterator>
  inline float readFloat(InputIterator& in) const {
    return ReadLeFloat(in);
  }

  /// Reads a little-endian IEEE754 double.
  template <typename InputIterator>
  inline double readDouble(InputIterator& in) const {
    return ReadLeDouble(in);
  }
};
#endif  // ROO_IO_IEEE754

/// Reads a byte-order-selected unsigned 16-bit integer from `in`.
template <typename InputIterator, ByteOrder byte_order>
constexpr uint16_t ReadU16(InputIterator& in) {
  return IntegerReader<byte_order>().readU16(in);
}

/// Reads a byte-order-selected unsigned 24-bit integer from `in`.
template <typename InputIterator, ByteOrder byte_order>
constexpr uint32_t ReadU24(InputIterator& in) {
  return IntegerReader<byte_order>().readU24(in);
}

/// Reads a byte-order-selected unsigned 32-bit integer from `in`.
template <typename InputIterator, ByteOrder byte_order>
constexpr uint32_t ReadU32(InputIterator& in) {
  return IntegerReader<byte_order>().readU32(in);
}

/// Reads a byte-order-selected unsigned 64-bit integer from `in`.
template <typename InputIterator, ByteOrder byte_order>
constexpr uint64_t ReadU64(InputIterator& in) {
  return IntegerReader<byte_order>().readU64(in);
}

#if ROO_IO_IEEE754
/// Reads a byte-order-selected IEEE754 float from `in`.
template <typename InputIterator, ByteOrder byte_order>
inline float ReadFloat(InputIterator& in) {
  return FloatReader<byte_order>().readFloat(in);
}

/// Reads a byte-order-selected IEEE754 double from `in`.
template <typename InputIterator, ByteOrder byte_order>
inline double ReadDouble(InputIterator& in) {
  return FloatReader<byte_order>().readDouble(in);
}
#endif  // ROO_IO_IEEE754

/// Reads host-native trivially copyable values from an input iterator.
template <typename T>
struct HostNativeReader {
 public:
  /// Reads `T` from `in`, or returns `default_value` on short read.
  template <typename InputIterator>
  T read(InputIterator& in, T default_value = T()) const {
    T result;
    if (ReadByteArray(in, (byte*)&result, sizeof(result)) == sizeof(result)) {
      return result;
    }
    return default_value;
  }
};

/// Reads a portable length-prefixed string into `buf`.
///
/// `capacity` includes space for the terminating zero. If the encoded string
/// is longer than the available space, the result is truncated, but the entire
/// encoded string is still consumed from `in` so that subsequent reads remain
/// aligned. When `capacity` is non-zero, `buf` is always zero-terminated.
template <typename InputIterator>
size_t ReadCString(InputIterator& in, char* buf, size_t capacity = SIZE_MAX) {
  uint64_t len = ReadVarU64(in);
  if (in.status() != kOk) return 0;
  if (len + 1 <= capacity) {
    // Common case.
    size_t written = ReadByteArray(in, (byte*)buf, len);
    buf[written] = 0;
    return written;
  }
  if (capacity > 0) {
    size_t written = ReadByteArray(in, (byte*)buf, capacity - 1);
    buf[written] = 0;
    in.skip(len - written);
    return written;
  }
  in.skip(len);
  return 0;
}

/// Reads a portable length-prefixed string into a `std::string`.
///
/// If the encoded string exceeds `max_size`, the result is truncated to
/// `max_size`, but the iterator still consumes the full encoded string so that
/// subsequent reads can continue past it.
template <typename InputIterator>
std::string ReadString(InputIterator& in, size_t max_size = SIZE_MAX) {
  uint64_t len = ReadVarU64(in);
  if (in.status() != kOk) return "";
  std::string result;
  if (len <= max_size) {
    // Common case.
    result.reserve(len);
    for (size_t i = 0; i < len; ++i) {
      char ch = (char)ReadU8(in);
      if (in.status() != kOk) return result;
      result.push_back(ch);
    }
    return result;
  }
  result.reserve(max_size);
  for (size_t i = 0; i < max_size; ++i) {
    char ch = (char)ReadU8(in);
    if (in.status() != kOk) return result;
    result.push_back(ch);
  }
  in.skip(len - max_size);
  return result;
}

/// Reads a portable length-prefixed string view from a memory iterator.
///
/// The returned view points into the iterator's underlying memory buffer and
/// does not own the data. If the encoded string exceeds `max_size`, the view
/// is truncated, but the iterator still skips the full encoded string.
template <typename InputIterator,
          typename std::enable_if<
              internal::MemoryIteratorTraits<InputIterator>::is_memory,
              bool>::type = true>
roo::string_view ReadStringView(InputIterator& in, size_t max_size = SIZE_MAX) {
  uint64_t len = ReadVarU64(in);
  if (in.status() != kOk) return "";
  typename InputIterator::PtrType start = in.ptr();
  if (len <= max_size) {
    // Common case.
    in.skip(len);
    return roo::string_view((const char*)start, in.ptr() - start);
  }
  in.skip(max_size);
  roo::string_view result((const char*)start, in.ptr() - start);
  in.skip(len - max_size);
  return result;
}

}  // namespace roo_io
