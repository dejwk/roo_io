#pragma once

#include <stdint.h>

#include <cstring>
#include <limits>

#include "roo_io/base/byte.h"
#include "roo_io/data/byte_order.h"
#include "roo_io/data/ieee754.h"

namespace roo_io {

// Basic, unsigned.

/// Stores an unsigned 8-bit integer to `target`.
inline void StoreU8(uint8_t v, byte* target) { target[0] = (byte)v; }

/// Stores a big-endian unsigned 16-bit integer to `target`.
inline void StoreBeU16(uint16_t v, byte* target) {
  target[0] = (byte)(v >> 8);
  target[1] = (byte)(v >> 0);
}

/// Stores a little-endian unsigned 16-bit integer to `target`.
inline void StoreLeU16(uint16_t v, byte* target) {
  target[0] = (byte)(v >> 0);
  target[1] = (byte)(v >> 8);
}

/// Stores a big-endian unsigned 24-bit integer to `target`.
inline void StoreBeU24(uint32_t v, byte* target) {
  target[0] = (byte)(v >> 16);
  target[1] = (byte)(v >> 8);
  target[2] = (byte)(v >> 0);
}

/// Stores a little-endian unsigned 24-bit integer to `target`.
inline void StoreLeU24(uint32_t v, byte* target) {
  target[0] = (byte)(v >> 0);
  target[1] = (byte)(v >> 8);
  target[2] = (byte)(v >> 16);
}

/// Stores a big-endian unsigned 32-bit integer to `target`.
inline void StoreBeU32(uint32_t v, byte* target) {
  target[0] = (byte)(v >> 24);
  target[1] = (byte)(v >> 16);
  target[2] = (byte)(v >> 8);
  target[3] = (byte)(v >> 0);
}

/// Stores a little-endian unsigned 32-bit integer to `target`.
inline void StoreLeU32(uint32_t v, byte* target) {
  target[0] = (byte)(v >> 0);
  target[1] = (byte)(v >> 8);
  target[2] = (byte)(v >> 16);
  target[3] = (byte)(v >> 24);
}

/// Stores a big-endian unsigned 64-bit integer to `target`.
inline void StoreBeU64(uint64_t v, byte* target) {
  target[0] = (byte)(v >> 56);
  target[1] = (byte)(v >> 48);
  target[2] = (byte)(v >> 40);
  target[3] = (byte)(v >> 32);
  target[4] = (byte)(v >> 24);
  target[5] = (byte)(v >> 16);
  target[6] = (byte)(v >> 8);
  target[7] = (byte)(v >> 0);
}

/// Stores a little-endian unsigned 64-bit integer to `target`.
inline void StoreLeU64(uint64_t v, byte* target) {
  target[0] = (byte)(v >> 0);
  target[1] = (byte)(v >> 8);
  target[2] = (byte)(v >> 16);
  target[3] = (byte)(v >> 24);
  target[4] = (byte)(v >> 32);
  target[5] = (byte)(v >> 40);
  target[6] = (byte)(v >> 48);
  target[7] = (byte)(v >> 56);
}

// Basic, signed.

/// Stores a signed 8-bit integer to `target`.
inline void StoreS8(int8_t v, byte* target) { StoreU8(v, target); }

/// Stores a big-endian signed 16-bit integer to `target`.
inline void StoreBeS16(uint16_t v, byte* target) { StoreBeU16(v, target); }

/// Stores a little-endian signed 16-bit integer to `target`.
inline void StoreLeS16(uint16_t v, byte* target) { StoreLeU16(v, target); }

/// Stores a big-endian signed 24-bit integer to `target`.
inline void StoreBeS24(uint32_t v, byte* target) { StoreBeU24(v, target); }

/// Stores a little-endian signed 24-bit integer to `target`.
inline void StoreLeS24(uint32_t v, byte* target) { StoreLeU24(v, target); }

/// Stores a big-endian signed 32-bit integer to `target`.
inline void StoreBeS32(uint32_t v, byte* target) { StoreBeU32(v, target); }

/// Stores a little-endian signed 32-bit integer to `target`.
inline void StoreLeS32(uint32_t v, byte* target) { StoreLeU32(v, target); }

/// Stores a big-endian signed 64-bit integer to `target`.
inline void StoreBeS64(uint64_t v, byte* target) { StoreBeU64(v, target); }

/// Stores a little-endian signed 64-bit integer to `target`.
inline void StoreLeS64(uint64_t v, byte* target) { StoreLeU64(v, target); }

#if ROO_IO_IEEE754
/// Stores a big-endian IEEE754 float into the first 4 bytes at `target`.
///
/// Requires host `float` to be a 32-bit IEEE754 type.
inline void StoreBeFloat(float v, byte* target) {
  static_assert(sizeof(float) == sizeof(uint32_t),
                "StoreBeFloat requires 32-bit float.");
  static_assert(std::numeric_limits<float>::is_iec559,
                "StoreBeFloat requires IEEE754 float.");
  uint32_t bits;
  memcpy(&bits, &v, sizeof(bits));
  StoreBeU32(bits, target);
}

/// Stores a little-endian IEEE754 float into the first 4 bytes at `target`.
///
/// Requires host `float` to be a 32-bit IEEE754 type.
inline void StoreLeFloat(float v, byte* target) {
  static_assert(sizeof(float) == sizeof(uint32_t),
                "StoreLeFloat requires 32-bit float.");
  static_assert(std::numeric_limits<float>::is_iec559,
                "StoreLeFloat requires IEEE754 float.");
  uint32_t bits;
  memcpy(&bits, &v, sizeof(bits));
  StoreLeU32(bits, target);
}

/// Stores a big-endian IEEE754 double into the first 8 bytes at `target`.
///
/// Requires host `double` to be a 64-bit IEEE754 type.
inline void StoreBeDouble(double v, byte* target) {
  static_assert(sizeof(double) == sizeof(uint64_t),
                "StoreBeDouble requires 64-bit double.");
  static_assert(std::numeric_limits<double>::is_iec559,
                "StoreBeDouble requires IEEE754 double.");
  uint64_t bits;
  memcpy(&bits, &v, sizeof(bits));
  StoreBeU64(bits, target);
}

/// Stores a little-endian IEEE754 double into the first 8 bytes at `target`.
///
/// Requires host `double` to be a 64-bit IEEE754 type.
inline void StoreLeDouble(double v, byte* target) {
  static_assert(sizeof(double) == sizeof(uint64_t),
                "StoreLeDouble requires 64-bit double.");
  static_assert(std::numeric_limits<double>::is_iec559,
                "StoreLeDouble requires IEEE754 double.");
  uint64_t bits;
  memcpy(&bits, &v, sizeof(bits));
  StoreLeU64(bits, target);
}
#endif  // ROO_IO_IEEE754

// Arbitrary types, native encoding.

/// Stores a host-native trivially copyable value to `target`.
///
/// `target` must point to at least `sizeof(T)` writable bytes. The bytes are
/// written using the host byte order and object representation, so this is
/// only portable between compatible platforms and is typically paired with
/// `LoadHostNative()`.
template <typename T>
inline void StoreHostNative(const T& v, byte* target) {
  memcpy((char*)target, (const char*)&v, sizeof(v));
}

// Variants that can be used in code templated on byte order.

/// Stores a byte-order-selected unsigned 16-bit integer into the first 2 bytes
/// at `target`.
template <ByteOrder byte_order>
inline void StoreU16(uint16_t v, byte* target);

/// Stores a byte-order-selected unsigned 24-bit integer into the first 3 bytes
/// at `target`.
template <ByteOrder byte_order>
inline void StoreU24(uint32_t v, byte* target);

/// Stores a byte-order-selected unsigned 32-bit integer into the first 4 bytes
/// at `target`.
template <ByteOrder byte_order>
inline void StoreU32(uint32_t v, byte* target);

/// Stores a byte-order-selected unsigned 64-bit integer into the first 8 bytes
/// at `target`.
template <ByteOrder byte_order>
inline void StoreU64(uint64_t v, byte* target);

/// Stores a byte-order-selected signed 16-bit integer into the first 2 bytes
/// at `target`.
template <ByteOrder byte_order>
inline void StoreS16(int16_t v, byte* target);

/// Stores a byte-order-selected signed 24-bit integer into the first 3 bytes
/// at `target`.
template <ByteOrder byte_order>
inline void StoreS24(int32_t v, byte* target);

/// Stores a byte-order-selected signed 32-bit integer into the first 4 bytes
/// at `target`.
template <ByteOrder byte_order>
inline void StoreS32(int32_t v, byte* target);

/// Stores a byte-order-selected signed 64-bit integer into the first 8 bytes
/// at `target`.
template <ByteOrder byte_order>
inline void StoreS64(int64_t v, byte* target);

#if ROO_IO_IEEE754
/// Stores a byte-order-selected IEEE754 float into the first 4 bytes at
/// `target`.
///
/// Requires host `float` to be a 32-bit IEEE754 type.
template <ByteOrder byte_order>
inline void StoreFloat(float v, byte* target);

/// Stores a byte-order-selected IEEE754 double into the first 8 bytes at
/// `target`.
///
/// Requires host `double` to be a 64-bit IEEE754 type.
template <ByteOrder byte_order>
inline void StoreDouble(double v, byte* target);
#endif  // ROO_IO_IEEE754

// Templated on both byte order and the type.

/// Stores a byte-order-selected integer of type `IntegerType` to `target`.
template <ByteOrder byte_order, typename IntegerType>
inline void StoreInteger(IntegerType v, byte* target);

// Implementation details.

template <>
inline void StoreU16<kBigEndian>(uint16_t v, byte* target) {
  StoreBeU16(v, target);
}

template <>
inline void StoreU16<kLittleEndian>(uint16_t v, byte* target) {
  StoreLeU16(v, target);
}

template <>
inline void StoreU24<kBigEndian>(uint32_t v, byte* target) {
  StoreBeU24(v, target);
}

template <>
inline void StoreU24<kLittleEndian>(uint32_t v, byte* target) {
  StoreLeU24(v, target);
}

template <>
inline void StoreU32<kBigEndian>(uint32_t v, byte* target) {
  StoreBeU32(v, target);
}

template <>
inline void StoreU32<kLittleEndian>(uint32_t v, byte* target) {
  StoreLeU32(v, target);
}

template <>
inline void StoreU64<kBigEndian>(uint64_t v, byte* target) {
  StoreBeU64(v, target);
}

template <>
inline void StoreU64<kLittleEndian>(uint64_t v, byte* target) {
  StoreLeU64(v, target);
}

template <>
inline void StoreS16<kBigEndian>(int16_t v, byte* target) {
  StoreBeS16(v, target);
}

template <>
inline void StoreS16<kLittleEndian>(int16_t v, byte* target) {
  StoreLeS16(v, target);
}

template <>
inline void StoreS24<kBigEndian>(int32_t v, byte* target) {
  StoreBeS24(v, target);
}

template <>
inline void StoreS24<kLittleEndian>(int32_t v, byte* target) {
  StoreLeS24(v, target);
}

template <>
inline void StoreS32<kBigEndian>(int32_t v, byte* target) {
  StoreBeS32(v, target);
}

template <>
inline void StoreS32<kLittleEndian>(int32_t v, byte* target) {
  StoreLeS32(v, target);
}

template <>
inline void StoreS64<kBigEndian>(int64_t v, byte* target) {
  StoreBeS64(v, target);
}

template <>
inline void StoreS64<kLittleEndian>(int64_t v, byte* target) {
  StoreLeS64(v, target);
}

#if ROO_IO_IEEE754
template <>
inline void StoreFloat<kBigEndian>(float v, byte* target) {
  StoreBeFloat(v, target);
}

template <>
inline void StoreFloat<kLittleEndian>(float v, byte* target) {
  StoreLeFloat(v, target);
}

template <>
inline void StoreDouble<kBigEndian>(double v, byte* target) {
  StoreBeDouble(v, target);
}

template <>
inline void StoreDouble<kLittleEndian>(double v, byte* target) {
  StoreLeDouble(v, target);
}
#endif  // ROO_IO_IEEE754

template <>
inline void StoreInteger<kBigEndian, uint8_t>(uint8_t v, byte* target) {
  StoreU8(v, target);
}

template <>
inline void StoreInteger<kLittleEndian, uint8_t>(uint8_t v, byte* target) {
  StoreU8(v, target);
}

template <>
inline void StoreInteger<kBigEndian, int8_t>(int8_t v, byte* target) {
  StoreS8(v, target);
}

template <>
inline void StoreInteger<kLittleEndian, int8_t>(int8_t v, byte* target) {
  StoreS8(v, target);
}

template <>
inline void StoreInteger<kBigEndian, uint16_t>(uint16_t v, byte* target) {
  StoreBeU16(v, target);
}

template <>
inline void StoreInteger<kLittleEndian, uint16_t>(uint16_t v, byte* target) {
  StoreLeU16(v, target);
}

template <>
inline void StoreInteger<kBigEndian, uint32_t>(uint32_t v, byte* target) {
  StoreBeU32(v, target);
}

template <>
inline void StoreInteger<kLittleEndian, uint32_t>(uint32_t v, byte* target) {
  StoreLeU32(v, target);
}

template <>
inline void StoreInteger<kBigEndian, uint64_t>(uint64_t v, byte* target) {
  StoreBeU64(v, target);
}

template <>
inline void StoreInteger<kLittleEndian, uint64_t>(uint64_t v, byte* target) {
  StoreLeU64(v, target);
}

}  // namespace roo_io
