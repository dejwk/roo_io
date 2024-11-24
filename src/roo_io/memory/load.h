#pragma once

#include <stdint.h>

namespace roo_io {

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
