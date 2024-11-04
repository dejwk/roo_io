#pragma once

#include <stdint.h>

namespace roo_io {

inline void StoreU16be(uint16_t v, uint8_t *target) {
  target[0] = (v >> 8);
  target[1] = (v >> 0) & 0xFF;
}

inline void StoreU16le(uint16_t v, uint8_t *target) {
  target[0] = (v >> 0) & 0xFF;
  target[1] = (v >> 8);
}

inline void StoreU24be(uint32_t v, uint8_t *target) {
  target[0] = (v >> 16);
  target[1] = (v >> 8) & 0xFF;
  target[2] = (v >> 0) & 0xFF;
}

inline void StoreU24le(uint32_t v, uint8_t *target) {
  target[0] = (v >> 0) & 0xFF;
  target[1] = (v >> 8) & 0xFF;
  target[2] = (v >> 16);
}

inline void StoreU32be(uint32_t v, uint8_t *target) {
  target[0] = (v >> 24);
  target[1] = (v >> 16) & 0xFF;
  target[2] = (v >> 8) & 0xFF;
  target[3] = (v >> 0) & 0xFF;
}

inline void StoreU32le(uint32_t v, uint8_t *target) {
  target[0] = (v >> 0) & 0xFF;
  target[1] = (v >> 8) & 0xFF;
  target[2] = (v >> 16) & 0xFF;
  target[3] = (v >> 24);
}

inline const uint16_t LoadU16be(const uint8_t *source) {
  return (source[0] << 8) | (source[1] << 0);
}

inline const uint16_t LoadU16le(const uint8_t *source) {
  return (source[0] << 0) | (source[1] << 8);
}

inline const uint32_t LoadU24be(const uint8_t *source) {
  return (source[0] << 16) | (source[1] << 8) | (source[2] << 0);
}

inline const uint32_t LoadU24le(const uint8_t *source) {
  return (source[0] << 0) | (source[1] << 8) | (source[2] << 16);
}

inline const uint32_t LoadU32be(const uint8_t *source) {
  return (source[0] << 24) | (source[1] << 16) | (source[2] << 8) |
         (source[3] << 0);
}

inline const uint32_t LoadU32le(const uint8_t *source) {
  return (source[0] << 0) | (source[1] << 8) | (source[2] << 16) |
         (source[3] << 24);
}

}  // namespace roo_io
