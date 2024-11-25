#pragma once

#include <stdint.h>
#include <cstring>

namespace roo_io {

inline void StoreBeU16(uint16_t v, uint8_t *target) {
  target[0] = (v >> 8);
  target[1] = (v >> 0) & 0xFF;
}

inline void StoreLeU16(uint16_t v, uint8_t *target) {
  target[0] = (v >> 0) & 0xFF;
  target[1] = (v >> 8);
}

inline void StoreBeU24(uint32_t v, uint8_t *target) {
  target[0] = (v >> 16);
  target[1] = (v >> 8) & 0xFF;
  target[2] = (v >> 0) & 0xFF;
}

inline void StoreLeU24(uint32_t v, uint8_t *target) {
  target[0] = (v >> 0) & 0xFF;
  target[1] = (v >> 8) & 0xFF;
  target[2] = (v >> 16);
}

inline void StoreBeU32(uint32_t v, uint8_t *target) {
  target[0] = (v >> 24);
  target[1] = (v >> 16) & 0xFF;
  target[2] = (v >> 8) & 0xFF;
  target[3] = (v >> 0) & 0xFF;
}

inline void StoreLeU32(uint32_t v, uint8_t *target) {
  target[0] = (v >> 0) & 0xFF;
  target[1] = (v >> 8) & 0xFF;
  target[2] = (v >> 16) & 0xFF;
  target[3] = (v >> 24);
}

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

inline void StoreBeS16(uint16_t v, uint8_t *target) {
  StoreBeU16(v, target);
}

inline void StoreLeS16(uint16_t v, uint8_t *target) {
  StoreLeU16(v, target);
}

inline void StoreBeS24(uint32_t v, uint8_t *target) {
  StoreBeU24(v, target);
}

inline void StoreLeS24(uint32_t v, uint8_t *target) {
  StoreLeU24(v, target);
}

inline void StoreBeS32(uint32_t v, uint8_t *target) {
  StoreBeU32(v, target);
}

inline void StoreLeS32(uint32_t v, uint8_t *target) {
  StoreLeU32(v, target);
}

inline void StoreBeS64(uint64_t v, uint8_t *target) {
  StoreBeU64(v, target);
}

inline void StoreLeS64(uint64_t v, uint8_t *target) {
  StoreLeU64(v, target);
}

template <typename T>
inline void StoreHostNative(const T& v, uint8_t *target) {
  memcpy((char*)target, (const char*)&v, sizeof(v));
}

}  // namespace roo_io
