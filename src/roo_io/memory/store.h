#pragma once

#include <stdint.h>

#include <cstring>

#include "roo_io/data/byte_order.h"

namespace roo_io {

// Basic, unsigned.

// Stores a big-endian unsigned 16-bit int to the specified memory address.
inline void StoreBeU16(uint16_t v, uint8_t *target) {
  target[0] = (v >> 8);
  target[1] = (v >> 0) & 0xFF;
}

// Stores a little-endian unsigned 16-bit int to the specified memory address.
inline void StoreLeU16(uint16_t v, uint8_t *target) {
  target[0] = (v >> 0) & 0xFF;
  target[1] = (v >> 8);
}

// Stores a big-endian unsigned 24-bit int to the specified memory address.
inline void StoreBeU24(uint32_t v, uint8_t *target) {
  target[0] = (v >> 16);
  target[1] = (v >> 8) & 0xFF;
  target[2] = (v >> 0) & 0xFF;
}

// Stores a little-endian unsigned 24-bit int to the specified memory address.
inline void StoreLeU24(uint32_t v, uint8_t *target) {
  target[0] = (v >> 0) & 0xFF;
  target[1] = (v >> 8) & 0xFF;
  target[2] = (v >> 16);
}

// Stores a big-endian unsigned 32-bit int to the specified memory address.
inline void StoreBeU32(uint32_t v, uint8_t *target) {
  target[0] = (v >> 24);
  target[1] = (v >> 16) & 0xFF;
  target[2] = (v >> 8) & 0xFF;
  target[3] = (v >> 0) & 0xFF;
}

// Stores a little-endian unsigned 32-bit int to the specified memory address.
inline void StoreLeU32(uint32_t v, uint8_t *target) {
  target[0] = (v >> 0) & 0xFF;
  target[1] = (v >> 8) & 0xFF;
  target[2] = (v >> 16) & 0xFF;
  target[3] = (v >> 24);
}

// Stores a big-endian unsigned 64-bit int to the specified memory address.
inline void StoreBeU64(uint64_t v, uint8_t *target) {
  target[0] = (v >> 56);
  target[1] = (v >> 48) & 0xFF;
  target[2] = (v >> 40) & 0xFF;
  target[3] = (v >> 32) & 0xFF;
  target[4] = (v >> 24) & 0xFF;
  target[5] = (v >> 16) & 0xFF;
  target[6] = (v >> 8) & 0xFF;
  target[7] = (v >> 0) & 0xFF;
}

// Stores a little-endian unsigned 64-bit int to the specified memory address.
inline void StoreLeU64(uint64_t v, uint8_t *target) {
  target[0] = (v >> 0) & 0xFF;
  target[1] = (v >> 8) & 0xFF;
  target[2] = (v >> 16) & 0xFF;
  target[3] = (v >> 24) & 0xFF;
  target[4] = (v >> 32) & 0xFF;
  target[5] = (v >> 40) & 0xFF;
  target[6] = (v >> 48) & 0xFF;
  target[7] = (v >> 56);
}

// Basic, signed.

// Stores a big-endian signed 16-bit int to the specified memory address.
inline void StoreBeS16(uint16_t v, uint8_t *target) { StoreBeU16(v, target); }

// Stores a little-endian signed 16-bit int to the specified memory address.
inline void StoreLeS16(uint16_t v, uint8_t *target) { StoreLeU16(v, target); }

// Stores a big-endian signed 24-bit int to the specified memory address.
inline void StoreBeS24(uint32_t v, uint8_t *target) { StoreBeU24(v, target); }

// Stores a little-endian signed 24-bit int to the specified memory address.
inline void StoreLeS24(uint32_t v, uint8_t *target) { StoreLeU24(v, target); }

// Stores a big-endian signed 32-bit int to the specified memory address.
inline void StoreBeS32(uint32_t v, uint8_t *target) { StoreBeU32(v, target); }

// Stores a little-endian signed 32-bit int to the specified memory address.
inline void StoreLeS32(uint32_t v, uint8_t *target) { StoreLeU32(v, target); }

// Stores a big-endian signed 64-bit int to the specified memory address.
inline void StoreBeS64(uint64_t v, uint8_t *target) { StoreBeU64(v, target); }

// Stores a little-endian signed 64-bit int to the specified memory address.
inline void StoreLeS64(uint64_t v, uint8_t *target) { StoreLeU64(v, target); }

// Arbitrary types, native encoding.


// Stores a platform-native (implementation-dependent) datum to the specified
// memory address. T must be default-constructible and have trivial destructor.
template <typename T>
inline void StoreHostNative(const T &v, uint8_t *target) {
  memcpy((char *)target, (const char *)&v, sizeof(v));
}

// Variants that can be used in code templated on byte order.

// Stores an unsigned 16-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreU16(uint16_t v, uint8_t *target);

// Stores an unsigned 24-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreU24(uint32_t v, uint8_t *target);

// Stores an unsigned 32-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreU32(uint32_t v, uint8_t *target);

// Stores an unsigned 64-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreU64(uint64_t v, uint8_t *target);

// Stores a signed 16-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreS16(int16_t v, uint8_t *target);

// Stores a signed 24-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreS24(int32_t v, uint8_t *target);

// Stores a signed 32-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreS32(int32_t v, uint8_t *target);

// Stores a signed 64-bit int to the specified memory address.
template <ByteOrder byte_order>
inline void StoreS64(int64_t v, uint8_t *target);

// Templated on both byte order and the type.

// Stores an integer to the specified memory address.
template <ByteOrder byte_order, typename IntegerType>
inline void StoreInteger(IntegerType v, uint8_t *target);



// Implementation details.

template <>
inline void StoreU16<kBigEndian>(uint16_t v, uint8_t *target) {
  StoreBeU16(v, target);
}

template <>
inline void StoreU16<kLittleEndian>(uint16_t v, uint8_t *target) {
  StoreLeU16(v, target);
}

template <>
inline void StoreU24<kBigEndian>(uint32_t v, uint8_t *target) {
  StoreBeU24(v, target);
}

template <>
inline void StoreU24<kLittleEndian>(uint32_t v, uint8_t *target) {
  StoreLeU24(v, target);
}

template <>
inline void StoreU32<kBigEndian>(uint32_t v, uint8_t *target) {
  StoreBeU32(v, target);
}

template <>
inline void StoreU32<kLittleEndian>(uint32_t v, uint8_t *target) {
  StoreLeU32(v, target);
}

template <>
inline void StoreU64<kBigEndian>(uint64_t v, uint8_t *target) {
  StoreBeU64(v, target);
}

template <>
inline void StoreU64<kLittleEndian>(uint64_t v, uint8_t *target) {
  StoreLeU64(v, target);
}

template <>
inline void StoreS16<kBigEndian>(int16_t v, uint8_t *target) {
  StoreBeS16(v, target);
}

template <>
inline void StoreS16<kLittleEndian>(int16_t v, uint8_t *target) {
  StoreLeS16(v, target);
}

template <>
inline void StoreS24<kBigEndian>(int32_t v, uint8_t *target) {
  StoreBeS24(v, target);
}

template <>
inline void StoreS24<kLittleEndian>(int32_t v, uint8_t *target) {
  StoreLeS24(v, target);
}

template <>
inline void StoreS32<kBigEndian>(int32_t v, uint8_t *target) {
  StoreBeS32(v, target);
}

template <>
inline void StoreS32<kLittleEndian>(int32_t v, uint8_t *target) {
  StoreLeS32(v, target);
}

template <>
inline void StoreS64<kBigEndian>(int64_t v, uint8_t *target) {
  StoreBeS64(v, target);
}

template <>
inline void StoreS64<kLittleEndian>(int64_t v, uint8_t *target) {
  StoreLeS64(v, target);
}

template <>
inline void StoreInteger<kBigEndian, uint8_t>(uint8_t v, uint8_t *target) {
  *target = v;
}

template <>
inline void StoreInteger<kLittleEndian, int8_t>(int8_t v, uint8_t *target) {
  *target = v;
}

template <>
inline void StoreInteger<kBigEndian, uint16_t>(uint16_t v, uint8_t *target) {
  StoreBeU16(v, target);
}

template <>
inline void StoreInteger<kLittleEndian, uint16_t>(uint16_t v, uint8_t *target) {
  StoreLeU16(v, target);
}

template <>
inline void StoreInteger<kBigEndian, uint32_t>(uint32_t v, uint8_t *target) {
  StoreBeU32(v, target);
}

template <>
inline void StoreInteger<kLittleEndian, uint32_t>(uint32_t v, uint8_t *target) {
  StoreLeU32(v, target);
}

template <>
inline void StoreInteger<kBigEndian, uint64_t>(uint64_t v, uint8_t *target) {
  StoreBeU64(v, target);
}

template <>
inline void StoreInteger<kLittleEndian, uint64_t>(uint64_t v, uint8_t *target) {
  StoreLeU64(v, target);
}

}  // namespace roo_io
