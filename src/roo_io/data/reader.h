#pragma once

#include "roo_io/data/byte_order.h"
#include "roo_io/iterator/input_iterator.h"

namespace roo_io {

// Helper to read various data structures from consecutive bytes, assuming a
// specified byte order.
template <int byte_order>
class DataReader;

template <>
class DataReader<BIG_ENDIAN> {
 public:
  template <typename InputIterator>
  constexpr uint16_t read_u16(InputIterator& in) const {
    return ((uint16_t)in.read() << 8) | ((uint16_t)in.read() << 0);
  }

  template <typename InputIterator>
  constexpr uint32_t read_u24(InputIterator& in) const {
    return ((uint32_t)in.read() << 16) | ((uint32_t)in.read() << 8) |
           ((uint32_t)in.read() << 0);
  }

  template <typename InputIterator>
  constexpr uint32_t read_u32(InputIterator& in) const {
    return ((uint32_t)in.read() << 24) | ((uint32_t)in.read() << 16) |
           ((uint32_t)in.read() << 8) | ((uint32_t)in.read() << 0);
  }

  template <typename InputIterator>
  constexpr uint32_t read_u64(InputIterator& in) const {
    return ((uint64_t)read_u32(in) << 32) | ((uint64_t)read_u32(in) << 0);
  }

  template <typename InputIterator>
  constexpr float read_float(InputIterator& in) const {
    return *reinterpret_cast<float*>(reinterpret_cast<char*>(&read_u32(in)));
  }
};

template <>
class DataReader<LITTLE_ENDIAN> {
 public:
  template <typename InputIterator>
  constexpr uint16_t read_u16(InputIterator& in) const {
    return ((uint16_t)in.read() << 0) | ((uint16_t)in.read() << 8);
  }

  template <typename InputIterator>
  constexpr uint32_t read_u24(InputIterator& in) const {
    return ((uint32_t)in.read() << 0) | ((uint32_t)in.read() << 8) |
           ((uint32_t)in.read() << 16);
  }

  template <typename InputIterator>
  constexpr uint32_t read_u32(InputIterator& in) const {
    return ((uint32_t)in.read() << 0) | ((uint32_t)in.read() << 8) |
           ((uint32_t)in.read() << 16) | ((uint32_t)in.read() << 24);
  }

  template <typename InputIterator>
  constexpr uint32_t read_u64(InputIterator& in) const {
    return ((uint64_t)read_u32(in) << 0) | ((uint64_t)read_u32(in) << 32);
  }

  template <typename InputIterator>
  constexpr float read_float(InputIterator& in) const {
    return *reinterpret_cast<float*>(reinterpret_cast<char*>(&read_u32(in)));
  }
};

template <typename InputIterator, int byte_order>
constexpr uint16_t ReadU16(InputIterator& in) {
  return DataReader<byte_order>().read_u16(in);
}

template <typename InputIterator, int byte_order>
constexpr uint32_t ReadU24(InputIterator& in) {
  return DataReader<byte_order>().read_u24(in);
}

template <typename InputIterator, int byte_order>
constexpr uint32_t ReadU32(InputIterator& in) {
  return DataReader<byte_order>().read_u32(in);
}

// Convenience helper to read network-encoded (big-endian) uint16.
template <typename InputIterator>
constexpr uint16_t ReadU16be(InputIterator& in) {
  return DataReader<BIG_ENDIAN>().read_u16(in);
}

// Convenience helper to read network-encoded (big-endian) uint24.
template <typename InputIterator>
constexpr uint32_t ReadU24be(InputIterator& in) {
  return DataReader<BIG_ENDIAN>().read_u24(in);
}

// Convenience helper to read network-encoded (big-endian) uint32.
template <typename InputIterator>
constexpr uint32_t ReadU32be(InputIterator& in) {
  return DataReader<BIG_ENDIAN>().read_u32(in);
}

template <typename InputIterator>
unsigned int ReadByteArray(InputIterator& in, uint8_t* result,
                           unsigned int count) {
  unsigned int read_total = 0;
  while (count > 0) {
    int read_now = in.read(result, count);
    if (read_now <= 0) break;
    result += read_now;
    read_total += read_now;
    count -= read_now;
  }
  return read_total;
}

template <typename InputIterator>
uint64_t ReadVarU64(InputIterator& in) {
  uint64_t result = 0;
  int read;
  int shift = 0;
  do {
    read = in.get();
    if (in.status() != kOk) {
      return 0;
    }
    result |= ((uint64_t)(read & 0x7F) << shift);
    shift += 7;
  } while ((read & 0x80) != 0);
  return result;
}

}  // namespace roo_io
