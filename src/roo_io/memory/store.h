#pragma once

#include <stdint.h>

#include <cstring>

#include "roo_io/base/byte.h"
#include "roo_io/data/byte_order.h"

namespace roo_io {

// Basic, unsigned.

// Stores an unsigned 8-bit int to the specified memory address.
inline void StoreU8(uint8_t v, byte* target) { target[0] = (byte)v; }

// Stores a big-endian unsigned 16-bit int to the specified memory address.
inline void StoreBeU16(uint16_t v, byte* target) {
  target[0] = (byte)(v >> 8);
  target[1] = (byte)(v >> 0);
}

// Stores a little-endian unsigned 16-bit int to the specified memory address.
inline void StoreLeU16(uint16_t v, byte* target) {
  target[0] = (byte)(v >> 0);
  target[1] = (byte)(v >> 8);
}

// Stores a big-endian unsigned 24-bit int to the specified memory address.
inline void StoreBeU24(uint32_t v, byte* target) {
  target[0] = (byte)(v >> 16);
  target[1] = (byte)(v >> 8);
  target[2] = (byte)(v >> 0);
}

// Stores a little-endian unsigned 24-bit int to the specified memory address.
inline void StoreLeU24(uint32_t v, byte* target) {
  target[0] = (byte)(v >> 0);
  target[1] = (byte)(v >> 8);
  target[2] = (byte)(v >> 16);
}

// Stores a big-endian unsigned 32-bit int to the specified memory address.
inline void StoreBeU32(uint32_t v, byte* target) {
  target[0] = (byte)(v >> 24);
  target[1] = (byte)(v >> 16);
  target[2] = (byte)(v >> 8);
  target[3] = (byte)(v >> 0);
}

// Stores a little-endian unsigned 32-bit int to the specified memory address.
inline void StoreLeU32(uint32_t v, byte* target) {
  target[0] = (byte)(v >> 0);
  target[1] = (byte)(v >> 8);
  target[2] = (byte)(v >> 16);
  target[3] = (byte)(v >> 24);
}

// Stores a big-endian unsigned 64-bit int to the specified memory address.
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

// Stores a little-endian unsigned 64-bit int to the specified memory address.
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

// Stores an unsigned 8-bit int to the specified memory address.
inline void StoreS8(int8_t v, byte* target) { StoreU8(v, target); }

// Stores a big-endian signed 16-bit int to the specified memory address.
inline void StoreBeS16(uint16_t v, byte* target) { StoreBeU16(v, target); }

// Stores a little-endian signed 16-bit int to the specified memory address.
inline void StoreLeS16(uint16_t v, byte* target) { StoreLeU16(v, target); }

// Stores a big-endian signed 24-bit int to the specified memory address.
inline void StoreBeS24(uint32_t v, byte* target) { StoreBeU24(v, target); }

// Stores a little-endian signed 24-bit int to the specified memory address.
inline void StoreLeS24(uint32_t v, byte* target) { StoreLeU24(v, target); }

// Stores a big-endian signed 32-bit int to the specified memory address.
inline void StoreBeS32(uint32_t v, byte* target) { StoreBeU32(v, target); }

// Stores a little-endian signed 32-bit int to the specified memory address.
inline void StoreLeS32(uint32_t v, byte* target) { StoreLeU32(v, target); }

// Stores a big-endian signed 64-bit int to the specified memory address.
inline void StoreBeS64(uint64_t v, byte* target) { StoreBeU64(v, target); }

// Stores a little-endian signed 64-bit int to the specified memory address.
inline void StoreLeS64(uint64_t v, byte* target) { StoreLeU64(v, target); }

// Arbitrary types, native encoding.

// Stores a platform-native (implementation-dependent) datum to the specified
// memory address. T must be default-constructible and have trivial destructor.
template <typename T>
inline void StoreHostNative(const T& v, byte* target) {
  memcpy((char*)target, (const char*)&v, sizeof(v));
}

// Variants that can be used in code templated on byte order.

// Stores an unsigned 16-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreU16(uint16_t v, byte* target);

// Stores an unsigned 24-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreU24(uint32_t v, byte* target);

// Stores an unsigned 32-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreU32(uint32_t v, byte* target);

// Stores an unsigned 64-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreU64(uint64_t v, byte* target);

// Stores a signed 16-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreS16(int16_t v, byte* target);

// Stores a signed 24-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreS24(int32_t v, byte* target);

// Stores a signed 32-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreS32(int32_t v, byte* target);

// Stores a signed 64-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreS64(int64_t v, byte* target);

// Templated on both byte order and the type.

// Stores an integer to the specified memory address.
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

template <>
inline void StoreInteger<kBigEndian, uint8_t>(uint8_t v, byte* target) {
  StoreU8(v, target);
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
