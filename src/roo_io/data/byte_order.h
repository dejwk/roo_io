#pragma once

// Support for endian-ness conversions between platform-specific and
// platform-independent representations that can be used as template
// parameters.

// Note: a decent compiler will completely remove 'no-op' conversions,
// and inline swaps using native builtins.

#include <inttypes.h>
#include "roo_io/third_party/endianness.h"

namespace roo_io {

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

// Templated operator for swapping the byte order of any unsigned integer type.
template <typename type>
struct Swapper;

template <>
struct Swapper<uint8_t> {
  constexpr uint8_t operator()(uint8_t in) const { return in; }
};

template <>
struct Swapper<uint16_t> {
  constexpr uint16_t operator()(uint16_t in) const {
    return bswap16(in);
  }
};

template <>
struct Swapper<uint32_t> {
  constexpr uint32_t operator()(uint32_t in) const {
    return bswap32(in);
  }
};

template <>
struct Swapper<uint64_t> {
  constexpr uint64_t operator()(uint64_t in) const {
    return bswap64(in);
  }
};

template <typename storage_type>
constexpr storage_type Swap(storage_type in) {
  return Swapper<storage_type>()(in);
}

// Operator for converting the byte order from src to dst. The default
// implementation, that assumes that src != dst, and thus swapping is needed.
template <typename storage_type, ByteOrder src, ByteOrder dst>
struct Converter {
  constexpr storage_type operator()(storage_type in) const {
    return Swap(in);
  }
};

// No swapping if src == dst.
template <typename storage_type, ByteOrder byte_order>
struct Converter<storage_type, byte_order, byte_order> {
  constexpr storage_type operator()(storage_type in) const { return in; }
};

// Utility to convert an integer type from the src to dst byte order.
// If src == dst, compiles to no-op; otherwise, it compiles to
// a built-in byte swap routine.
template <typename storage_type, ByteOrder src, ByteOrder dst>
constexpr storage_type Convert(storage_type in) {
  return Converter<storage_type, src, dst>()(in);
}

}  // namespace byte_order

// A convenience function that swaps the bytes in the integer type.
template <typename storage_type>
constexpr storage_type bswap(storage_type in) {
  return byte_order::Swap(in);
}

// A convenience function to convert an integer type from
// host-native byte order to the specified byte order.
template <typename storage_type, ByteOrder dst>
constexpr storage_type hto(storage_type in) {
  return byte_order::Convert<storage_type, kNativeEndian, dst>(in);
}

// A convenience function to convert an integer type from
// a specified byte order to the host-native byte order.
template <typename storage_type, ByteOrder src>
constexpr storage_type toh(storage_type in) {
  return byte_order::Convert<storage_type, src, kNativeEndian>(in);
}

// A convenience function to convert an integer type from
// host-native byte order to big endian.
template <typename storage_type>
constexpr storage_type htobe(storage_type in) {
  return byte_order::Convert<storage_type, kNativeEndian, kBigEndian>(in);
}

// A convenience function to convert an integer type from
// host-native byte order to little endian.
template <typename storage_type>
constexpr storage_type htole(storage_type in) {
  return byte_order::Convert<storage_type, kNativeEndian, kLittleEndian>(in);
}

// A convenience function to convert an integer type from
// big endian to the host-native byte order.
template <typename storage_type>
constexpr storage_type betoh(storage_type in) {
  return byte_order::Convert<storage_type, kBigEndian, kNativeEndian>(in);
}

// A convenience function to convert an integer type from
// little endian to the host-native byte order.
template <typename storage_type>
constexpr storage_type letoh(storage_type in) {
  return byte_order::Convert<storage_type, kLittleEndian, kNativeEndian>(in);
}

}  // namespace roo_io
