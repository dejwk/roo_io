#pragma once

#include <cstring>
#include <limits>
#include <type_traits>

#include "roo_backport.h"
#include "roo_backport/string_view.h"
#include "roo_io/core/input_iterator.h"
#include "roo_io/data/byte_order.h"
#include "roo_io/data/ieee754.h"
#include "roo_io/data/zigzag.h"
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

/// Reads a checked protobuf-style variable-length unsigned 64-bit integer.
///
/// Returns false without changing `value` when the input ends, has an error,
/// or contains an unrepresentable varint. A false result with `in.status() ==
/// kOk` denotes malformed input; otherwise the iterator status identifies the
/// input failure. A malformed varint consumes at most ten bytes.
template <typename InputIterator>
bool ReadVarU64(InputIterator& in, uint64_t& value) {
  if (in.status() != kOk) return false;
  // Keep variable shifts in 32-bit registers. Byte five spans both words;
  // byte ten has only one representable payload bit.
  uint32_t low = 0;
  for (unsigned shift = 0; shift < 28; shift += 7) {
    uint32_t read = static_cast<uint32_t>(in.read());
    if (in.status() != kOk) return false;
    low |= (read & 0x7F) << shift;
    if ((read & 0x80) == 0) {
      value = low;
      return true;
    }
  }
  uint32_t read = static_cast<uint32_t>(in.read());
  if (in.status() != kOk) return false;
  low |= (read & 0x0F) << 28;
  uint32_t high = (read & 0x70) >> 4;
  if ((read & 0x80) == 0) {
    value = (static_cast<uint64_t>(high) << 32) | low;
    return true;
  }
  for (unsigned shift = 3; shift <= 31; shift += 7) {
    read = static_cast<uint32_t>(in.read());
    if (in.status() != kOk) return false;
    if (shift == 31 && read > 1) return false;
    high |= (read & 0x7F) << shift;
    if ((read & 0x80) == 0) {
      value = (static_cast<uint64_t>(high) << 32) | low;
      return true;
    }
  }
  return false;
}

/// Reads a protobuf-style variable-length unsigned 64-bit integer from `in`.
///
/// This legacy overload returns zero on malformed input or input failure. Use
/// the checked output-argument overload when decoding untrusted input.
template <typename InputIterator>
[[deprecated("Use ReadVarU64(input, value)")]] uint64_t ReadVarU64(
    InputIterator& in) {
  uint64_t value = 0;
  ReadVarU64(in, value);
  return value;
}

/// Reads a checked protobuf-style variable-length unsigned 32-bit integer.
///
/// Returns false without changing `value` when the input cannot be decoded as
/// a representable unsigned 32-bit value.
template <typename InputIterator>
bool ReadVarU32(InputIterator& in, uint32_t& value) {
  uint64_t decoded = 0;
  if (!ReadVarU64(in, decoded)) return false;
  if (decoded > std::numeric_limits<uint32_t>::max()) return false;
  value = static_cast<uint32_t>(decoded);
  return true;
}

/// Reads a checked ZigZag-encoded signed 32-bit integer from `in`.
template <typename InputIterator>
bool ReadZigZag32(InputIterator& in, int32_t& value) {
  uint32_t encoded = 0;
  if (!ReadVarU32(in, encoded)) return false;
  value = ZigZagDecode32(encoded);
  return true;
}

/// Reads a checked ZigZag-encoded signed 64-bit integer from `in`.
template <typename InputIterator>
bool ReadZigZag64(InputIterator& in, int64_t& value) {
  uint64_t encoded = 0;
  if (!ReadVarU64(in, encoded)) return false;
  value = ZigZagDecode64(encoded);
  return true;
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

/// Reads a strict portable length-prefixed C string into `buf`.
///
/// `capacity` includes the trailing NUL and must be nonzero. Returns false
/// without consuming input when `capacity` is zero. On other failures, `buf`
/// remains NUL-terminated and `len` and payload contents are unspecified.
template <typename InputIterator>
bool ReadCString(InputIterator& in, char* buf, size_t capacity, size_t* len) {
  if (capacity == 0) return false;
  buf[0] = 0;
  uint64_t encoded_length = 0;
  if (!ReadVarU64(in, encoded_length)) return false;
  if (encoded_length >= capacity) return false;
  size_t read = ReadByteArray(in, reinterpret_cast<byte*>(buf),
                              static_cast<size_t>(encoded_length));
  buf[read] = 0;
  if (read != encoded_length) return false;
  *len = read;
  return true;
}

/// Reads a portable length-prefixed C string into `buf`, retaining what fits.
///
/// `capacity` includes the trailing NUL and must be nonzero. Oversized fields
/// succeed after their complete payload has been consumed.
template <typename InputIterator>
bool ReadCStringTruncated(InputIterator& in, char* buf, size_t capacity,
                          size_t* len) {
  if (capacity == 0) return false;
  buf[0] = 0;
  uint64_t encoded_length = 0;
  if (!ReadVarU64(in, encoded_length)) return false;
  size_t retained = encoded_length < capacity - 1
                        ? static_cast<size_t>(encoded_length)
                        : capacity - 1;
  size_t read = ReadByteArray(in, reinterpret_cast<byte*>(buf), retained);
  buf[read] = 0;
  if (read != retained) return false;
  uint64_t remaining = encoded_length - retained;
  while (remaining > 0) {
    size_t chunk =
        remaining > SIZE_MAX ? SIZE_MAX : static_cast<size_t>(remaining);
    in.skip(chunk);
    if (in.status() != kOk) return false;
    remaining -= chunk;
  }
  *len = retained;
  return true;
}

/// Reads a strict portable length-prefixed string into `output`.
///
/// Returns false before consuming the payload when its encoded length exceeds
/// `max_size` or `output->max_size()`.
template <typename InputIterator>
bool ReadString(InputIterator& in, std::string* output,
                size_t max_size = SIZE_MAX) {
  uint64_t encoded_length = 0;
  if (!ReadVarU64(in, encoded_length)) return false;
  if (encoded_length > max_size || encoded_length > output->max_size()) {
    return false;
  }
  size_t length = static_cast<size_t>(encoded_length);
  output->resize(length);
  if (length == 0) return true;
  return ReadByteArray(in, reinterpret_cast<byte*>(&(*output)[0]), length) ==
         length;
}

/// Reads a portable length-prefixed string into `output`, retaining what fits.
///
/// An oversized field succeeds after its complete payload has been consumed.
template <typename InputIterator>
bool ReadStringTruncated(InputIterator& in, std::string* output,
                         size_t max_size = SIZE_MAX) {
  uint64_t encoded_length = 0;
  if (!ReadVarU64(in, encoded_length)) return false;
  uint64_t retained_length =
      encoded_length < max_size ? encoded_length : max_size;
  if (retained_length > output->max_size()) return false;
  size_t retained = static_cast<size_t>(retained_length);
  output->resize(retained);
  if (retained > 0 && ReadByteArray(in, reinterpret_cast<byte*>(&(*output)[0]),
                                    retained) != retained) {
    return false;
  }
  uint64_t remaining = encoded_length - retained;
  while (remaining > 0) {
    size_t chunk =
        remaining > SIZE_MAX ? SIZE_MAX : static_cast<size_t>(remaining);
    in.skip(chunk);
    if (in.status() != kOk) return false;
    remaining -= chunk;
  }
  return true;
}

/// Reads a strict portable length-prefixed view from a memory iterator.
///
/// The returned view borrows the iterator's backing memory and is valid only
/// while that memory remains valid.
template <typename InputIterator,
          typename std::enable_if<
              internal::MemoryIteratorTraits<InputIterator>::is_memory,
              bool>::type = true>
bool ReadStringView(InputIterator& in, roo::string_view* output,
                    size_t max_size = SIZE_MAX) {
  uint64_t encoded_length = 0;
  if (!ReadVarU64(in, encoded_length)) return false;
  if (encoded_length > max_size) return false;
  typename InputIterator::PtrType start = in.ptr();
  size_t length = static_cast<size_t>(encoded_length);
  in.skip(length);
  if (in.status() != kOk) return false;
  *output = roo::string_view(reinterpret_cast<const char*>(start), length);
  return true;
}

}  // namespace roo_io
