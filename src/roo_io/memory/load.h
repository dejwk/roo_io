#pragma once

#include <cstdint>
#include <cstring>

#include "roo_io/data/byte_order.h"

namespace roo_io {

// Loads a big-endian unsigned 16-bit int from the specified memory address.
inline constexpr uint16_t LoadBeU16(const uint8_t *source) {
  return (source[0] << 8) | (source[1] << 0);
}

// Loads a little-endian unsigned 16-bit int from the specified memory address.
inline constexpr uint16_t LoadLeU16(const uint8_t *source) {
  return (source[0] << 0) | (source[1] << 8);
}

// Loads a big-endian unsigned 24-bit int from the specified memory address.
inline constexpr uint32_t LoadBeU24(const uint8_t *source) {
  return (source[0] << 16) | (source[1] << 8) | (source[2] << 0);
}

// Loads a little-endian unsigned 24-bit int from the specified memory address.
inline constexpr uint32_t LoadLeU24(const uint8_t *source) {
  return (source[0] << 0) | (source[1] << 8) | (source[2] << 16);
}

// Loads a big-endian unsigned 32-bit int from the specified memory address.
inline constexpr uint32_t LoadBeU32(const uint8_t *source) {
  return (source[0] << 24) | (source[1] << 16) | (source[2] << 8) |
         (source[3] << 0);
}

// Loads a little-endian unsigned 32-bit int from the specified memory address.
inline constexpr uint32_t LoadLeU32(const uint8_t *source) {
  return (source[0] << 0) | (source[1] << 8) | (source[2] << 16) |
         (source[3] << 24);
}

// Loads a big-endian unsigned 64-bit int from the specified memory address.
inline constexpr uint64_t LoadBeU64(const uint8_t *source) {
  return (((uint64_t)LoadBeU32(source)) << 32) | LoadBeU32(source + 4);
}

// Loads a little-endian unsigned 64-bit int from the specified memory address.
inline constexpr uint64_t LoadLeU64(const uint8_t *source) {
  return LoadLeU32(source) | (((uint64_t)LoadLeU32(source + 4)) << 32);
}

// Loads a big-endian signed 16-bit int from the specified memory address.
inline constexpr int16_t LoadBeS16(const uint8_t *source) {
  return (int16_t)LoadBeU16(source);
}

// Loads a little-endian signed 16-bit int from the specified memory address.
inline constexpr int16_t LoadLeS16(const uint8_t *source) {
  return (int16_t)LoadLeU16(source);
}

// Loads a big-endian signed 24-bit int from the specified memory address.
inline constexpr int32_t LoadBeS24(const uint8_t *source) {
  return (int32_t)LoadBeU24(source) | (((source[0] & 0x80) > 0) * 0xFF) << 24;
}

// Loads a little-endian signed 24-bit int from the specified memory address.
inline constexpr int32_t LoadLeS24(const uint8_t *source) {
  return (int32_t)LoadLeU24(source) | (((source[3] & 0x80) > 0) * 0xFF) << 24;
}

// Loads a big-endian signed 32-bit int from the specified memory address.
inline constexpr int32_t LoadBeS32(const uint8_t *source) {
  return (int32_t)LoadBeU32(source);
}

// Loads a little-endian signed 32-bit int from the specified memory address.
inline constexpr int32_t LoadLeS32(const uint8_t *source) {
  return (int32_t)LoadLeU32(source);
}

// Loads a big-endian signed 64-bit int from the specified memory address.
inline constexpr int64_t LoadBeS64(const uint8_t *source) {
  return (int64_t)LoadBeU64(source);
}

// Loads a little-endian signed 64-bit int from the specified memory address.
inline constexpr int64_t LoadLeS64(const uint8_t *source) {
  return (int64_t)LoadLeU64(source);
}

// Loads a platform-native (implementation-dependent) datum from the specified
// memory address. T must be default-constructible and have trivial destructor.
template <typename T>
inline constexpr T LoadHostNative(const uint8_t *source) {
  T result;
  memcpy(&result, source, sizeof(result));
  return result;
}

// Variants that can be used in code templated on the byte order.

// Loads an unsigned 16-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr uint16_t LoadU16(const uint8_t *source);

// Loads an unsigned 24-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr uint32_t LoadU24(const uint8_t *source);

// Loads an unsigned 32-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr uint32_t LoadU32(const uint8_t *source);

// Loads an unsigned 64-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr uint64_t LoadU64(const uint8_t *source);

// Loads a signed 16-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr int16_t LoadS16(const uint8_t *source);

// Loads a signed 24-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr int32_t LoadS24(const uint8_t *source);

// Loads a signed 32-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr int32_t LoadS32(const uint8_t *source);

// Loads a signed 64-bit int from the specified memory address.
template <ByteOrder byte_order>
inline constexpr int64_t LoadS64(const uint8_t *source);

// Variants that can be used in code templated on the storage type.

// Loads a big-endian integer from the specified memory address.
template <typename IntegerType>
inline constexpr IntegerType LoadBeInteger(const uint8_t *source);

// Loads a little-endian integer from the specified memory address.
template <typename IntegerType>
inline constexpr IntegerType LoadLeInteger(const uint8_t *source);

// Variant that can be used in code templated on both the byte order and the
// storage type.

// Loads an integer from the specified memory address.
template <ByteOrder byte_order, typename IntegerType>
inline constexpr IntegerType LoadInteger(const uint8_t *source);

// Implementation details below.

template <>
inline constexpr uint16_t LoadU16<kBigEndian>(const uint8_t *source) {
  return LoadBeU16(source);
}

template <>
inline constexpr uint16_t LoadU16<kLittleEndian>(const uint8_t *source) {
  return LoadLeU16(source);
}

template <>
inline constexpr uint32_t LoadU24<kBigEndian>(const uint8_t *source) {
  return LoadBeU24(source);
}

template <>
inline constexpr uint32_t LoadU24<kLittleEndian>(const uint8_t *source) {
  return LoadLeU24(source);
}

template <>
inline constexpr uint32_t LoadU32<kBigEndian>(const uint8_t *source) {
  return LoadBeU32(source);
}

template <>
inline constexpr uint32_t LoadU32<kLittleEndian>(const uint8_t *source) {
  return LoadLeU32(source);
}

template <>
inline constexpr uint64_t LoadU64<kBigEndian>(const uint8_t *source) {
  return LoadBeU64(source);
}

template <>
inline constexpr uint64_t LoadU64<kLittleEndian>(const uint8_t *source) {
  return LoadLeU64(source);
}

template <>
inline constexpr int16_t LoadS16<kBigEndian>(const uint8_t *source) {
  return LoadBeS16(source);
}

template <>
inline constexpr int16_t LoadS16<kLittleEndian>(const uint8_t *source) {
  return LoadLeS16(source);
}

template <>
inline constexpr int32_t LoadS24<kBigEndian>(const uint8_t *source) {
  return LoadBeS24(source);
}

template <>
inline constexpr int32_t LoadS24<kLittleEndian>(const uint8_t *source) {
  return LoadLeS24(source);
}

template <>
inline constexpr int32_t LoadS32<kBigEndian>(const uint8_t *source) {
  return LoadBeS32(source);
}

template <>
inline constexpr int32_t LoadS32<kLittleEndian>(const uint8_t *source) {
  return LoadLeS32(source);
}

template <>
inline constexpr int64_t LoadS64<kBigEndian>(const uint8_t *source) {
  return LoadBeS64(source);
}

template <>
inline constexpr int64_t LoadS64<kLittleEndian>(const uint8_t *source) {
  return LoadLeS64(source);
}

template <>
inline constexpr uint8_t LoadInteger<kBigEndian, uint8_t>(
    const uint8_t *source) {
  return *source;
}

template <>
inline constexpr uint8_t LoadInteger<kLittleEndian, uint8_t>(
    const uint8_t *source) {
  return *source;
}

template <>
inline constexpr uint16_t LoadInteger<kBigEndian, uint16_t>(
    const uint8_t *source) {
  return LoadBeU16(source);
}

template <>
inline constexpr uint16_t LoadInteger<kLittleEndian, uint16_t>(
    const uint8_t *source) {
  return LoadLeU16(source);
}

template <>
inline constexpr uint32_t LoadInteger<kBigEndian, uint32_t>(
    const uint8_t *source) {
  return LoadBeU32(source);
}

template <>
inline constexpr uint32_t LoadInteger<kLittleEndian, uint32_t>(
    const uint8_t *source) {
  return LoadLeU32(source);
}

template <>
inline constexpr uint64_t LoadInteger<kBigEndian, uint64_t>(
    const uint8_t *source) {
  return LoadBeU64(source);
}

template <>
inline constexpr uint64_t LoadInteger<kLittleEndian, uint64_t>(
    const uint8_t *source) {
  return LoadLeU64(source);
}

template <>
inline constexpr int8_t LoadInteger<kBigEndian, int8_t>(
    const uint8_t *source) {
  return (int8_t)*source;
}

template <>
inline constexpr int8_t LoadInteger<kLittleEndian, int8_t>(
    const uint8_t *source) {
  return (int8_t)*source;
}

template <>
inline constexpr int16_t LoadInteger<kBigEndian, int16_t>(
    const uint8_t *source) {
  return LoadBeS16(source);
}

template <>
inline constexpr int16_t LoadInteger<kLittleEndian, int16_t>(
    const uint8_t *source) {
  return LoadLeS16(source);
}

template <>
inline constexpr int32_t LoadInteger<kBigEndian, int32_t>(
    const uint8_t *source) {
  return LoadBeS32(source);
}

template <>
inline constexpr int32_t LoadInteger<kLittleEndian, int32_t>(
    const uint8_t *source) {
  return LoadLeS32(source);
}

template <>
inline constexpr int64_t LoadInteger<kBigEndian, int64_t>(
    const uint8_t *source) {
  return LoadBeS64(source);
}

template <>
inline constexpr int64_t LoadInteger<kLittleEndian, int64_t>(
    const uint8_t *source) {
  return LoadLeS64(source);
}

template <>
inline constexpr uint8_t LoadBeInteger<uint8_t>(const uint8_t *source) {
  return *source;
}

template <>
inline constexpr uint8_t LoadLeInteger<uint8_t>(const uint8_t *source) {
  return *source;
}

template <>
inline constexpr uint16_t LoadBeInteger<uint16_t>(const uint8_t *source) {
  return LoadBeU16(source);
}

template <>
inline constexpr uint16_t LoadLeInteger<uint16_t>(const uint8_t *source) {
  return LoadLeU16(source);
}

template <>
inline constexpr uint32_t LoadBeInteger<uint32_t>(const uint8_t *source) {
  return LoadBeU32(source);
}

template <>
inline constexpr uint32_t LoadLeInteger<uint32_t>(const uint8_t *source) {
  return LoadLeU32(source);
}

template <>
inline constexpr uint64_t LoadBeInteger<uint64_t>(const uint8_t *source) {
  return LoadBeU64(source);
}

template <>
inline constexpr uint64_t LoadLeInteger<uint64_t>(const uint8_t *source) {
  return LoadLeU64(source);
}

template <>
inline constexpr int8_t LoadBeInteger<int8_t>(const uint8_t *source) {
  return (int8_t)*source;
}

template <>
inline constexpr int8_t LoadLeInteger<int8_t>(const uint8_t *source) {
  return (int8_t)*source;
}

template <>
inline constexpr int16_t LoadBeInteger<int16_t>(const uint8_t *source) {
  return LoadBeS16(source);
}

template <>
inline constexpr int16_t LoadLeInteger<int16_t>(const uint8_t *source) {
  return LoadLeS16(source);
}

template <>
inline constexpr int32_t LoadBeInteger<int32_t>(const uint8_t *source) {
  return LoadBeS32(source);
}

template <>
inline constexpr int32_t LoadLeInteger<int32_t>(const uint8_t *source) {
  return LoadLeS32(source);
}

template <>
inline constexpr int64_t LoadBeInteger<int64_t>(const uint8_t *source) {
  return LoadBeS64(source);
}

template <>
inline constexpr int64_t LoadLeInteger<int64_t>(const uint8_t *source) {
  return LoadLeS64(source);
}

}  // namespace roo_io
