#pragma once

#include "roo_io/data/byte_order.h"
#include "roo_io/iterator/input_iterator.h"

namespace roo_io {

template <typename InputIterator>
constexpr uint16_t ReadBeU16(InputIterator& in) {
  return ((uint16_t)in.read() << 8) | ((uint16_t)in.read() << 0);
}

template <typename InputIterator>
constexpr uint16_t ReadLeU16(InputIterator& in) {
  return ((uint16_t)in.read() << 0) | ((uint16_t)in.read() << 8);
}

template <typename InputIterator>
constexpr uint32_t ReadBeU24(InputIterator& in) {
  return ((uint32_t)in.read() << 16) | ((uint32_t)in.read() << 8) |
         ((uint32_t)in.read() << 0);
}

template <typename InputIterator>
constexpr uint32_t ReadLeU24(InputIterator& in) {
  return ((uint32_t)in.read() << 0) | ((uint32_t)in.read() << 8) |
         ((uint32_t)in.read() << 16);
}

template <typename InputIterator>
constexpr uint32_t ReadBeU32(InputIterator& in) {
  return ((uint32_t)in.read() << 24) | ((uint32_t)in.read() << 16) |
         ((uint32_t)in.read() << 8) | ((uint32_t)in.read() << 0);
}

template <typename InputIterator>
constexpr uint32_t ReadLeU32(InputIterator& in) {
  return ((uint32_t)in.read() << 0) | ((uint32_t)in.read() << 8) |
         ((uint32_t)in.read() << 16) | ((uint32_t)in.read() << 24);
}

template <typename InputIterator>
constexpr uint32_t ReadBeU64(InputIterator& in) {
  return ((uint64_t)ReadBeU32(in) << 32) | ReadBeU32(in);
}

template <typename InputIterator>
constexpr uint32_t ReadLeU64(InputIterator& in) {
  return ReadBeU32(in) | ((uint64_t)ReadLeU32(in) << 32);
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
    read = in.read();
    if (in.status() != kOk) {
      return 0;
    }
    result |= ((uint64_t)(read & 0x7F) << shift);
    shift += 7;
  } while ((read & 0x80) != 0);
  return result;
}

// Helper to read numbers templated on the byte order.
template <ByteOrder byte_order>
class NumberReader;

template <>
class NumberReader<kBigEndian> {
 public:
  template <typename InputIterator>
  constexpr uint16_t readU16(InputIterator& in) const {
    return ReadBeU16(in);
  }

  template <typename InputIterator>
  constexpr uint32_t readU24(InputIterator& in) const {
    return ReadBeU24(in);
  }

  template <typename InputIterator>
  constexpr uint32_t readU32(InputIterator& in) const {
    return ReadBeU32(in);
  }

  template <typename InputIterator>
  constexpr uint32_t readU64(InputIterator& in) const {
    return ReadBeU64(in);
  }

  // template <typename InputIterator>
  // constexpr float read_float(InputIterator& in) const {
  //   return *reinterpret_cast<float*>(reinterpret_cast<char*>(&read_u32(in)));
  // }
};

template <>
class NumberReader<kLittleEndian> {
 public:
  template <typename InputIterator>
  constexpr uint16_t ReadU16(InputIterator& in) const {
    return ReadLeU16(in);
  }

  template <typename InputIterator>
  constexpr uint32_t read_u24(InputIterator& in) const {
    return ReadLeU24(in);
  }

  template <typename InputIterator>
  constexpr uint32_t read_u32(InputIterator& in) const {
    return ReadLeU32(in);
  }

  template <typename InputIterator>
  constexpr uint32_t read_u64(InputIterator& in) const {
    return ReadLeU64(in);
  }

  // template <typename InputIterator>
  // constexpr float read_float(InputIterator& in) const {
  //   return *reinterpret_cast<float*>(reinterpret_cast<char*>(&read_u32(in)));
  // }
};

template <typename InputIterator, ByteOrder byte_order>
constexpr uint16_t ReadU16(InputIterator& in) {
  return NumberReader<byte_order>().readU16(in);
}

template <typename InputIterator, ByteOrder byte_order>
constexpr uint32_t ReadU24(InputIterator& in) {
  return NumberReader<byte_order>().readU24(in);
}

template <typename InputIterator, ByteOrder byte_order>
constexpr uint32_t ReadU32(InputIterator& in) {
  return NumberReader<byte_order>().readU32(in);
}

}  // namespace roo_io
