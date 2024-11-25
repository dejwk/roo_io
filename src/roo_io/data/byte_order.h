#pragma once

// Support for endian-ness conversions between platform-specific and
// platform-independent representations that can be used as template
// parameters.

// The 'integer' template parameters are meant to take values defined in
// <endian.h>, e.g. BYTE_ORDER, LITTLE_ENDIAN, BIG_ENDIAN.
//
// Note: a decent compiler will completely remove 'no-op' conversions,
// and inline swaps using native builtins.

#include <endian.h>
#include <inttypes.h>

namespace roo_io {

namespace byte_order {

// Operator for swapping the byte order.
template <typename type>
struct Swapper;

template <>
struct Swapper<uint8_t> {
  constexpr uint8_t operator()(uint8_t in) const { return in; }
};

template <>
struct Swapper<uint16_t> {
  constexpr uint16_t operator()(uint16_t in) const {
    return BYTE_ORDER == LITTLE_ENDIAN ? htobe16(in) : htole16(in);
  }
};

template <>
struct Swapper<uint64_t> {
  constexpr uint64_t operator()(uint64_t in) const {
    return BYTE_ORDER == LITTLE_ENDIAN ? htobe64(in) : htole64(in);
  }
};

template <>
struct Swapper<uint32_t> {
  constexpr uint32_t operator()(uint32_t in) const {
    return BYTE_ORDER == LITTLE_ENDIAN ? htobe32(in) : htole32(in);
  }
};

// Operator for converting the byte order from src to dst. The default
// implementation, that assumes that src != dst, and thus swapping is needed.
template <typename storage_type, int byte_order_src, int byte_order_dst>
struct Converter {
  constexpr storage_type operator()(storage_type in) const {
    return Swapper<storage_type>()(in);
  }
};

// No swapping if src == dst.
template <typename storage_type, int byte_order>
struct Converter<storage_type, byte_order, byte_order> {
  constexpr storage_type operator()(storage_type in) const { return in; }
};

// Utility to convert an integer type from the src to dst byte order.
// If src == dst, compiles to no-op; otherwise, it compiles to
// a built-in byte swap routine.
template <typename storage_type, int byte_order_src, int byte_order_dst>
constexpr storage_type Convert(storage_type in) {
  return Converter<storage_type, byte_order_src, byte_order_dst>()(in);
}

}  // namespace byte_order

// A convenience function that swaps the bytes in the integer type.
template <typename storage_type>
constexpr storage_type bswap(storage_type in) {
  return byte_order::Swapper<storage_type>()(in);
}

// A convenience function to convert an integer type from
// host-native byte order to the specified byte order.
template <typename storage_type, int byte_order_dst>
constexpr storage_type hto(storage_type in) {
  return byte_order::Convert<storage_type, BYTE_ORDER, byte_order_dst>(in);
}

// A convenience function to convert an integer type from
// a specified byte order to the host-native byte order.
template <typename storage_type, int byte_order_src>
constexpr storage_type toh(storage_type in) {
  return byte_order::Convert<storage_type, byte_order_src, BYTE_ORDER>(in);
}

// A convenience function to convert an integer type from
// host-native byte order to big endian.
template <typename storage_type>
constexpr storage_type htobe(storage_type in) {
  return byte_order::Convert<storage_type, BYTE_ORDER, BIG_ENDIAN>(in);
}

// A convenience function to convert an integer type from
// host-native byte order to little endian.
template <typename storage_type>
constexpr storage_type htole(storage_type in) {
  return byte_order::Convert<storage_type, BYTE_ORDER, LITTLE_ENDIAN>(in);
}

// A convenience function to convert an integer type from
// big endian to the host-native byte order.
template <typename storage_type>
constexpr storage_type betoh(storage_type in) {
  return byte_order::Convert<storage_type, BIG_ENDIAN, BYTE_ORDER>(in);
}

// A convenience function to convert an integer type from
// little endian to the host-native byte order.
template <typename storage_type>
constexpr storage_type letoh(storage_type in) {
  return byte_order::Convert<storage_type, LITTLE_ENDIAN, BYTE_ORDER>(in);
}

}  // namespace roo_io
