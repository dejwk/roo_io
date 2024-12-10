#pragma once

#include <cstdint>
#include <cstring>

#include "roo_io/base/byte.h"
#include "roo_io/data/byte_order.h"

namespace roo_io {

inline constexpr uint8_t LoadU8(const byte *source) {
  return (uint8_t)(*source);
}

// Loads a big-endian unsigned 16-bit int from the specified memory address.
inline constexpr uint16_t LoadBeU16(const byte *source) {
  return ((uint8_t)source[0] << 8) | ((uint8_t)source[1] << 0);
}

// Loads a little-endian unsigned 16-bit int from the specified memory address.
inline constexpr uint16_t LoadLeU16(const byte *source) {
  return ((uint8_t)source[0] << 0) | ((uint8_t)source[1] << 8);
}

// Loads a big-endian unsigned 24-bit int from the specified memory address.
inline constexpr uint32_t LoadBeU24(const byte *source) {
  return ((uint8_t)source[0] << 16) | ((uint8_t)source[1] << 8) |
         ((uint8_t)source[2] << 0);
}

// Loads a little-endian unsigned 24-bit int from the specified memory address.
inline constexpr uint32_t LoadLeU24(const byte *source) {
  return ((uint8_t)source[0] << 0) | ((uint8_t)source[1] << 8) |
         ((uint8_t)source[2] << 16);
}

// Loads a big-endian unsigned 32-bit int from the specified memory address.
inline constexpr uint32_t LoadBeU32(const byte *source) {
  return ((uint8_t)source[0] << 24) | ((uint8_t)source[1] << 16) |
         ((uint8_t)source[2] << 8) | ((uint8_t)source[3] << 0);
}

// Loads a little-endian unsigned 32-bit int from the specified memory address.
inline constexpr uint32_t LoadLeU32(const byte *source) {
  return ((uint8_t)source[0] << 0) | ((uint8_t)source[1] << 8) |
         ((uint8_t)source[2] << 16) | ((uint8_t)source[3] << 24);
}

// Loads a big-endian unsigned 64-bit int from the specified memory address.
inline constexpr uint64_t LoadBeU64(const byte *source) {
  return (((uint64_t)LoadBeU32(source)) << 32) | LoadBeU32(source + 4);
}

// Loads a little-endian unsigned 64-bit int from the specified memory address.
inline constexpr uint64_t LoadLeU64(const byte *source) {
  return LoadLeU32(source) | (((uint64_t)LoadLeU32(source + 4)) << 32);
}

inline constexpr int8_t LoadS8(const byte *source) {
  return (int8_t)LoadU8(source);
}

// Loads a big-endian signed 16-bit int from the specified memory address.
inline constexpr int16_t LoadBeS16(const byte *source) {
  return (int16_t)LoadBeU16(source);
}

// Loads a little-endian signed 16-bit int from the specified memory address.
inline constexpr int16_t LoadLeS16(const byte *source) {
  return (int16_t)LoadLeU16(source);
}

// Loads a big-endian signed 24-bit int from the specified memory address.
inline constexpr int32_t LoadBeS24(const byte *source) {
  return (int32_t)LoadBeU24(source) | ((((uint8_t)source[0] & 0x80) > 0) * 0xFF)
                                          << 24;
}

// Loads a little-endian signed 24-bit int from the specified memory address.
inline constexpr int32_t LoadLeS24(const byte *source) {
  return (int32_t)LoadLeU24(source) | ((((uint8_t)source[3] & 0x80) > 0) * 0xFF)
                                          << 24;
}

// Loads a big-endian signed 32-bit int from the specified memory address.
inline constexpr int32_t LoadBeS32(const byte *source) {
  return (int32_t)LoadBeU32(source);
}

// Loads a little-endian signed 32-bit int from the specified memory address.
inline constexpr int32_t LoadLeS32(const byte *source) {
  return (int32_t)LoadLeU32(source);
}

// Loads a big-endian signed 64-bit int from the specified memory address.
inline constexpr int64_t LoadBeS64(const byte *source) {
  return (int64_t)LoadBeU64(source);
}

// Loads a little-endian signed 64-bit int from the specified memory address.
inline constexpr int64_t LoadLeS64(const byte *source) {
  return (int64_t)LoadLeU64(source);
}

// Loads a platform-native (implementation-dependent) datum from the specified
// memory address. T must be default-constructible and have trivial destructor.
template <typename T>
inline constexpr T LoadHostNative(const byte *source) {
  T result;
  memcpy(&result, (const byte *)source, sizeof(result));
  return result;
}

// Variants that can be used in code templated on the byte order.

// Loads an unsigned 16-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr uint16_t LoadU16(const byte *source);

// Loads an unsigned 24-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr uint32_t LoadU24(const byte *source);

// Loads an unsigned 32-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr uint32_t LoadU32(const byte *source);

// Loads an unsigned 64-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr uint64_t LoadU64(const byte *source);

// Loads a signed 16-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr int16_t LoadS16(const byte *source);

// Loads a signed 24-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr int32_t LoadS24(const byte *source);

// Loads a signed 32-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr int32_t LoadS32(const byte *source);

// Loads a signed 64-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr int64_t LoadS64(const byte *source);

// Variants that can be used in code templated on the storage type.

// Loads a big-endian integer from the specified memory address.
template <typename IntegerType>
inline constexpr IntegerType LoadBeInteger(const byte *source);

// Loads a little-endian integer from the specified memory address.
template <typename IntegerType>
inline constexpr IntegerType LoadLeInteger(const byte *source);

// Variant that can be used in code templated on both the byte order and the
// storage type.

// Loads an integer from the specified memory address.
template <ByteOrder byte_order, typename IntegerType>
inline constexpr IntegerType LoadInteger(const byte *source);

// Implementation details below.

template <>
inline constexpr uint16_t LoadU16<kBigEndian>(const byte *source) {
  return LoadBeU16(source);
}

template <>
inline constexpr uint16_t LoadU16<kLittleEndian>(const byte *source) {
  return LoadLeU16(source);
}

template <>
inline constexpr uint32_t LoadU24<kBigEndian>(const byte *source) {
  return LoadBeU24(source);
}

template <>
inline constexpr uint32_t LoadU24<kLittleEndian>(const byte *source) {
  return LoadLeU24(source);
}

template <>
inline constexpr uint32_t LoadU32<kBigEndian>(const byte *source) {
  return LoadBeU32(source);
}

template <>
inline constexpr uint32_t LoadU32<kLittleEndian>(const byte *source) {
  return LoadLeU32(source);
}

template <>
inline constexpr uint64_t LoadU64<kBigEndian>(const byte *source) {
  return LoadBeU64(source);
}

template <>
inline constexpr uint64_t LoadU64<kLittleEndian>(const byte *source) {
  return LoadLeU64(source);
}

template <>
inline constexpr int16_t LoadS16<kBigEndian>(const byte *source) {
  return LoadBeS16(source);
}

template <>
inline constexpr int16_t LoadS16<kLittleEndian>(const byte *source) {
  return LoadLeS16(source);
}

template <>
inline constexpr int32_t LoadS24<kBigEndian>(const byte *source) {
  return LoadBeS24(source);
}

template <>
inline constexpr int32_t LoadS24<kLittleEndian>(const byte *source) {
  return LoadLeS24(source);
}

template <>
inline constexpr int32_t LoadS32<kBigEndian>(const byte *source) {
  return LoadBeS32(source);
}

template <>
inline constexpr int32_t LoadS32<kLittleEndian>(const byte *source) {
  return LoadLeS32(source);
}

template <>
inline constexpr int64_t LoadS64<kBigEndian>(const byte *source) {
  return LoadBeS64(source);
}

template <>
inline constexpr int64_t LoadS64<kLittleEndian>(const byte *source) {
  return LoadLeS64(source);
}

template <>
inline constexpr uint8_t LoadInteger<kBigEndian, uint8_t>(const byte *source) {
  return LoadU8(source);
}

template <>
inline constexpr uint8_t LoadInteger<kLittleEndian, uint8_t>(
    const byte *source) {
  return LoadU8(source);
}

template <>
inline constexpr uint16_t LoadInteger<kBigEndian, uint16_t>(
    const byte *source) {
  return LoadBeU16(source);
}

template <>
inline constexpr uint16_t LoadInteger<kLittleEndian, uint16_t>(
    const byte *source) {
  return LoadLeU16(source);
}

template <>
inline constexpr uint32_t LoadInteger<kBigEndian, uint32_t>(
    const byte *source) {
  return LoadBeU32(source);
}

template <>
inline constexpr uint32_t LoadInteger<kLittleEndian, uint32_t>(
    const byte *source) {
  return LoadLeU32(source);
}

template <>
inline constexpr uint64_t LoadInteger<kBigEndian, uint64_t>(
    const byte *source) {
  return LoadBeU64(source);
}

template <>
inline constexpr uint64_t LoadInteger<kLittleEndian, uint64_t>(
    const byte *source) {
  return LoadLeU64(source);
}

template <>
inline constexpr int8_t LoadInteger<kBigEndian, int8_t>(const byte *source) {
  return (int8_t)*(const byte *)source;
}

template <>
inline constexpr int8_t LoadInteger<kLittleEndian, int8_t>(const byte *source) {
  return (int8_t)*(const byte *)source;
}

template <>
inline constexpr int16_t LoadInteger<kBigEndian, int16_t>(const byte *source) {
  return LoadBeS16(source);
}

template <>
inline constexpr int16_t LoadInteger<kLittleEndian, int16_t>(
    const byte *source) {
  return LoadLeS16(source);
}

template <>
inline constexpr int32_t LoadInteger<kBigEndian, int32_t>(const byte *source) {
  return LoadBeS32(source);
}

template <>
inline constexpr int32_t LoadInteger<kLittleEndian, int32_t>(
    const byte *source) {
  return LoadLeS32(source);
}

template <>
inline constexpr int64_t LoadInteger<kBigEndian, int64_t>(const byte *source) {
  return LoadBeS64(source);
}

template <>
inline constexpr int64_t LoadInteger<kLittleEndian, int64_t>(
    const byte *source) {
  return LoadLeS64(source);
}

template <>
inline constexpr uint8_t LoadBeInteger<uint8_t>(const byte *source) {
  return LoadU8(source);
}

template <>
inline constexpr uint8_t LoadLeInteger<uint8_t>(const byte *source) {
  return LoadU8(source);
}

template <>
inline constexpr uint16_t LoadBeInteger<uint16_t>(const byte *source) {
  return LoadBeU16(source);
}

template <>
inline constexpr uint16_t LoadLeInteger<uint16_t>(const byte *source) {
  return LoadLeU16(source);
}

template <>
inline constexpr uint32_t LoadBeInteger<uint32_t>(const byte *source) {
  return LoadBeU32(source);
}

template <>
inline constexpr uint32_t LoadLeInteger<uint32_t>(const byte *source) {
  return LoadLeU32(source);
}

template <>
inline constexpr uint64_t LoadBeInteger<uint64_t>(const byte *source) {
  return LoadBeU64(source);
}

template <>
inline constexpr uint64_t LoadLeInteger<uint64_t>(const byte *source) {
  return LoadLeU64(source);
}

template <>
inline constexpr int8_t LoadBeInteger<int8_t>(const byte *source) {
  return LoadS8(source);
}

template <>
inline constexpr int8_t LoadLeInteger<int8_t>(const byte *source) {
  return LoadS8(source);
}

template <>
inline constexpr int16_t LoadBeInteger<int16_t>(const byte *source) {
  return LoadBeS16(source);
}

template <>
inline constexpr int16_t LoadLeInteger<int16_t>(const byte *source) {
  return LoadLeS16(source);
}

template <>
inline constexpr int32_t LoadBeInteger<int32_t>(const byte *source) {
  return LoadBeS32(source);
}

template <>
inline constexpr int32_t LoadLeInteger<int32_t>(const byte *source) {
  return LoadLeS32(source);
}

template <>
inline constexpr int64_t LoadBeInteger<int64_t>(const byte *source) {
  return LoadBeS64(source);
}

template <>
inline constexpr int64_t LoadLeInteger<int64_t>(const byte *source) {
  return LoadLeS64(source);
}

}  // namespace roo_io
