#pragma once

#include <cstdint>

namespace roo_io {

/// Encodes a signed 32-bit value using the ZigZag transform.
constexpr uint32_t ZigZagEncode32(int32_t value) {
  uint32_t bits = static_cast<uint32_t>(value);
  return (bits << 1) ^ (0U - (bits >> 31));
}

/// Encodes a signed 64-bit value using the ZigZag transform.
constexpr uint64_t ZigZagEncode64(int64_t value) {
  uint64_t bits = static_cast<uint64_t>(value);
  return (bits << 1) ^ (0ULL - (bits >> 63));
}

/// Decodes a ZigZag-encoded 32-bit value.
constexpr int32_t ZigZagDecode32(uint32_t value) {
  return (value & 1U) == 0 ? static_cast<int32_t>(value >> 1)
                           : -1 - static_cast<int32_t>(value >> 1);
}

/// Decodes a ZigZag-encoded 64-bit value.
constexpr int64_t ZigZagDecode64(uint64_t value) {
  return (value & 1ULL) == 0 ? static_cast<int64_t>(value >> 1)
                             : -1 - static_cast<int64_t>(value >> 1);
}

}  // namespace roo_io
