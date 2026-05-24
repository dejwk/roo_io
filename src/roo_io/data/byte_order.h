#pragma once

// Support for endian-ness conversions between platform-specific and
// platform-independent representations that can be used as template
// parameters.

// Note: a decent compiler will completely remove 'no-op' conversions,
// and inline swaps using native builtins.

#include <inttypes.h>

#include "roo_io/third_party/endianness.h"

namespace roo_io {

/// Names the supported byte orders used by serialization helpers.
enum ByteOrder {
  kLittleEndian = 0,
  kBigEndian = 1,

#if defined(__BIG_ENDIAN__)
  kNativeEndian = kBigEndian,
#elif defined(__LITTLE_ENDIAN__)
  kNativeEndian = kLittleEndian,
#endif
};

}  // namespace roo_io

// Determine the native byte order if possible.

namespace roo_io {
namespace byte_order {

/// Swaps the byte order of an unsigned integer value.
template <typename type>
struct Swapper;

template <>
struct Swapper<uint8_t> {
  /// Returns `in` unchanged because 8-bit values have no byte order.
  constexpr uint8_t operator()(uint8_t in) const { return in; }
};

template <>
struct Swapper<uint16_t> {
  /// Returns `in` with its bytes reversed.
  constexpr uint16_t operator()(uint16_t in) const { return bswap16(in); }
};

template <>
struct Swapper<uint32_t> {
  /// Returns `in` with its bytes reversed.
  constexpr uint32_t operator()(uint32_t in) const { return bswap32(in); }
};

template <>
struct Swapper<uint64_t> {
  /// Returns `in` with its bytes reversed.
  constexpr uint64_t operator()(uint64_t in) const { return bswap64(in); }
};

/// Swaps the byte order of `in`.
template <typename storage_type>
constexpr storage_type Swap(storage_type in) {
  return Swapper<storage_type>()(in);
}

/// Converts integers from `src` byte order to `dst` byte order.
template <typename storage_type, ByteOrder src, ByteOrder dst>
struct Converter {
  /// Converts `in` by swapping because `src` and `dst` differ.
  constexpr storage_type operator()(storage_type in) const { return Swap(in); }
};

/// Specialization for the no-op conversion where source and destination match.
template <typename storage_type, ByteOrder byte_order>
struct Converter<storage_type, byte_order, byte_order> {
  /// Returns `in` unchanged.
  constexpr storage_type operator()(storage_type in) const { return in; }
};

/// Converts an integer from `src` byte order to `dst` byte order.
template <typename storage_type, ByteOrder src, ByteOrder dst>
constexpr storage_type Convert(storage_type in) {
  return Converter<storage_type, src, dst>()(in);
}

}  // namespace byte_order

/// Swaps the bytes in `in`.
template <typename storage_type>
constexpr storage_type bswap(storage_type in) {
  return byte_order::Swap(in);
}

/// Converts `in` from host-native byte order to `dst`.
template <typename storage_type, ByteOrder dst>
constexpr storage_type hto(storage_type in) {
  return byte_order::Convert<storage_type, kNativeEndian, dst>(in);
}

/// Converts `in` from `src` byte order to host-native byte order.
template <typename storage_type, ByteOrder src>
constexpr storage_type toh(storage_type in) {
  return byte_order::Convert<storage_type, src, kNativeEndian>(in);
}

/// Converts `in` from host-native byte order to big endian.
template <typename storage_type>
constexpr storage_type htobe(storage_type in) {
  return byte_order::Convert<storage_type, kNativeEndian, kBigEndian>(in);
}

/// Converts `in` from host-native byte order to little endian.
template <typename storage_type>
constexpr storage_type htole(storage_type in) {
  return byte_order::Convert<storage_type, kNativeEndian, kLittleEndian>(in);
}

/// Converts `in` from big endian to host-native byte order.
template <typename storage_type>
constexpr storage_type betoh(storage_type in) {
  return byte_order::Convert<storage_type, kBigEndian, kNativeEndian>(in);
}

/// Converts `in` from little endian to host-native byte order.
template <typename storage_type>
constexpr storage_type letoh(storage_type in) {
  return byte_order::Convert<storage_type, kLittleEndian, kNativeEndian>(in);
}

}  // namespace roo_io
