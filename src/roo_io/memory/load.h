#pragma once

#include <cstdint>
#include <cstring>

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

}  // namespace roo_io
