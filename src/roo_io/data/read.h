#pragma once

#include <type_traits>

#include "roo_io/base/string_view.h"
#include "roo_io/core/input_iterator.h"
#include "roo_io/data/byte_order.h"
#include "roo_io/memory/memory_input_iterator.h"

namespace roo_io {

// Unsigned.

// Reads an unsigned 8-bit int from the specified iterator.
template <typename InputIterator>
constexpr uint8_t ReadU8(InputIterator& in) {
  return (uint8_t)in.read();
}

// Reads a big-endian unsigned 16-bit int from the specified iterator.
template <typename InputIterator>
constexpr uint16_t ReadBeU16(InputIterator& in) {
  return ((uint16_t)in.read() << 8) | ((uint16_t)in.read() << 0);
}

// Reads a little-endian unsigned 16-bit int from the specified iterator.
template <typename InputIterator>
constexpr uint16_t ReadLeU16(InputIterator& in) {
  return ((uint16_t)in.read() << 0) | ((uint16_t)in.read() << 8);
}

// Reads a big-endian unsigned 24-bit int from the specified iterator.
template <typename InputIterator>
constexpr uint32_t ReadBeU24(InputIterator& in) {
  return ((uint32_t)in.read() << 16) | ((uint32_t)in.read() << 8) |
         ((uint32_t)in.read() << 0);
}

// Reads a little-endian unsigned 24-bit int from the specified iterator.
template <typename InputIterator>
constexpr uint32_t ReadLeU24(InputIterator& in) {
  return ((uint32_t)in.read() << 0) | ((uint32_t)in.read() << 8) |
         ((uint32_t)in.read() << 16);
}

// Reads a big-endian unsigned 32-bit int from the specified iterator.
template <typename InputIterator>
constexpr uint32_t ReadBeU32(InputIterator& in) {
  return ((uint32_t)in.read() << 24) | ((uint32_t)in.read() << 16) |
         ((uint32_t)in.read() << 8) | ((uint32_t)in.read() << 0);
}

// Reads a little-endian unsigned 32-bit int from the specified iterator.
template <typename InputIterator>
constexpr uint32_t ReadLeU32(InputIterator& in) {
  return ((uint32_t)in.read() << 0) | ((uint32_t)in.read() << 8) |
         ((uint32_t)in.read() << 16) | ((uint32_t)in.read() << 24);
}

// Reads a big-endian unsigned 64-bit int from the specified iterator.
template <typename InputIterator>
constexpr uint64_t ReadBeU64(InputIterator& in) {
  return ((uint64_t)ReadBeU32(in) << 32) | ReadBeU32(in);
}

// Reads a little-endian unsigned 64-bit int from the specified iterator.
template <typename InputIterator>
constexpr uint64_t ReadLeU64(InputIterator& in) {
  return ReadLeU32(in) | ((uint64_t)ReadLeU32(in) << 32);
}

// Signed.

// Reads a signed 8-bit int from the specified iterator.
template <typename InputIterator>
constexpr int8_t ReadS8(InputIterator& in) {
  return (int8_t)in.read();
}

// Reads a big-endian signed 16-bit int from the specified iterator.
template <typename InputIterator>
constexpr int16_t ReadBeS16(InputIterator& in) {
  return (int16_t)ReadBeU16(in);
}

// Reads a little-endian signed 16-bit int from the specified iterator.
template <typename InputIterator>
constexpr int16_t ReadLeS16(InputIterator& in) {
  return (int16_t)ReadLeU16(in);
}

namespace internal {

// Copies the sign bit (23th bit) to the top 8 bits.
constexpr int32_t sign_extend_24(int32_t v) {
  return v | (((v & 0x00800000) > 0) * 0xFF000000);
}

}  // namespace internal

// Reads a big-endian signed 24-bit int from the specified iterator.
template <typename InputIterator>
constexpr int32_t ReadBeS24(InputIterator& in) {
  return internal::sign_extend_24((int32_t)ReadBeU24(in));
}

// Reads a little-endian signed 24-bit int from the specified iterator.
template <typename InputIterator>
constexpr int32_t ReadLeS24(InputIterator& in) {
  return internal::sign_extend_24((int32_t)ReadLeU24(in));
}

// Reads a big-endian signed 32-bit int from the specified iterator.
template <typename InputIterator>
constexpr int32_t ReadBeS32(InputIterator& in) {
  return (int32_t)ReadBeU32(in);
}

// Reads a little-endian signed 32-bit int from the specified iterator.
template <typename InputIterator>
constexpr int32_t ReadLeS32(InputIterator& in) {
  return (int32_t)ReadLeU32(in);
}

// Reads a big-endian signed 64-bit int from the specified iterator.
template <typename InputIterator>
constexpr int64_t ReadBeS64(InputIterator& in) {
  return (int64_t)ReadBeU64(in);
}

// Reads a little-endian signed 64-bit int from the specified iterator.
template <typename InputIterator>
constexpr int64_t ReadLeS64(InputIterator& in) {
  return (int64_t)ReadLeU64(in);
}

// Reads `count` bytes from the input iterator, storing them in the result.
// Returns the number of bytes successfully read. If the returned value is
// smaller than `count`, it indicates that the end of stream has been reached,
// or that an error was encountered. The `status()` of the underlying iterator
// can be used to determine the cause.
template <typename InputIterator>
size_t ReadByteArray(InputIterator& in, byte* result, size_t count) {
  size_t read_total = 0;
  while (count > 0) {
    size_t read_now = in.read(result, count);
    if (read_now == 0) break;
    result += read_now;
    read_total += read_now;
    count -= read_now;
  }
  return read_total;
}

// Reads an unsigned 64-bit integer, encoded using variable-length encoding as
// defined by Google protocol buffers. (Small numbers take little space; numbers
// up to 127 take 1 byte).
template <typename InputIterator>
uint64_t ReadVarU64(InputIterator& in) {
  uint64_t result = 0;
  byte read;
  int shift = 0;
  do {
    read = in.read();
    if (in.status() != kOk) {
      return 0;
    }
    result |= ((uint64_t)(read & byte{0x7F}) << shift);
    shift += 7;
  } while ((read & byte{0x80}) != byte{0});
  return result;
}

// Helper to read integers templated on the byte order.
template <ByteOrder byte_order>
class IntegerReader;

template <>
class IntegerReader<kBigEndian> {
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
  constexpr uint64_t readU64(InputIterator& in) const {
    return ReadBeU64(in);
  }

  template <typename InputIterator>
  constexpr int16_t readS16(InputIterator& in) const {
    return ReadBeS16(in);
  }

  template <typename InputIterator>
  constexpr int32_t readS24(InputIterator& in) const {
    return ReadBeS24(in);
  }

  template <typename InputIterator>
  constexpr int32_t readS32(InputIterator& in) const {
    return ReadBeS32(in);
  }

  template <typename InputIterator>
  constexpr int64_t readS64(InputIterator& in) const {
    return ReadBeS64(in);
  }

  // template <typename InputIterator>
  // constexpr float read_float(InputIterator& in) const {
  //   return *reinterpret_cast<float*>(reinterpret_cast<char*>(&read_u32(in)));
  // }
};

template <>
class IntegerReader<kLittleEndian> {
 public:
  template <typename InputIterator>
  constexpr uint16_t readU16(InputIterator& in) const {
    return ReadLeU16(in);
  }

  template <typename InputIterator>
  constexpr uint32_t readU24(InputIterator& in) const {
    return ReadLeU24(in);
  }

  template <typename InputIterator>
  constexpr uint32_t readU32(InputIterator& in) const {
    return ReadLeU32(in);
  }

  template <typename InputIterator>
  constexpr uint64_t readU64(InputIterator& in) const {
    return ReadLeU64(in);
  }

  template <typename InputIterator>
  constexpr int16_t readS16(InputIterator& in) const {
    return ReadLeS16(in);
  }

  template <typename InputIterator>
  constexpr int32_t readS24(InputIterator& in) const {
    return ReadLeS24(in);
  }

  template <typename InputIterator>
  constexpr int32_t readS32(InputIterator& in) const {
    return ReadLeS32(in);
  }

  template <typename InputIterator>
  constexpr int64_t readS64(InputIterator& in) const {
    return ReadLeS64(in);
  }

  // template <typename InputIterator>
  // constexpr float read_float(InputIterator& in) const {
  //   return *reinterpret_cast<float*>(reinterpret_cast<char*>(&read_u32(in)));
  // }
};

template <typename InputIterator, ByteOrder byte_order>
constexpr uint16_t ReadU16(InputIterator& in) {
  return IntegerReader<byte_order>().readU16(in);
}

template <typename InputIterator, ByteOrder byte_order>
constexpr uint32_t ReadU24(InputIterator& in) {
  return IntegerReader<byte_order>().readU24(in);
}

template <typename InputIterator, ByteOrder byte_order>
constexpr uint32_t ReadU32(InputIterator& in) {
  return IntegerReader<byte_order>().readU32(in);
}

template <typename InputIterator, ByteOrder byte_order>
constexpr uint64_t ReadU64(InputIterator& in) {
  return IntegerReader<byte_order>().readU64(in);
}

// Allows reading platform-native (implementation-dependent) data from an input
// iterator. T must be default-constructible and have trivial destructor.
template <typename T>
struct HostNativeReader {
 public:
  // Reads T from the iterator. If T cannot be fully read, a default value is
  // returned.
  template <typename InputIterator>
  T read(InputIterator& in, T default_value = T()) const {
    T result;
    if (ReadByteArray(in, (byte*)&result, sizeof(result)) == sizeof(result)) {
      return result;
    }
    return default_value;
  }
};

// Reads a string, represented in portable representation (varint length
// followed by the character array), as a C string, into the specified buffer
// `buf` of specified `capacity`, and returns the length of the returned C
// string.
//
// If the string won't fit into the buffer, i.e. if the string's lengh plus 1
// (to account for the terminating zero) is greater than `capacity`, the
// returned string gets truncated. As long as `capacity` is greater than zero,
// the result gets zero-terminated. The return value indicates the actual
// returned length, not counting the terminal zero.
//
// Regardless whether the string gets truncated or not, it is always entirely
// skipped in the input stream, so that subsequent reads can access the data
// past the string.
template <typename InputIterator>
size_t ReadCString(InputIterator& in, char* buf, size_t capacity = SIZE_MAX) {
  uint64_t len = ReadVarU64(in);
  if (in.status() != kOk) return 0;
  if (len + 1 <= capacity) {
    // Common case.
    size_t written = ReadByteArray(in, (byte*)buf, len);
    buf[written] = 0;
    return written;
  }
  if (capacity > 0) {
    size_t written = ReadByteArray(in, (byte*)buf, capacity - 1);
    buf[written] = 0;
    in.skip(len - written);
    return written;
  }
  in.skip(len);
  return 0;
}

// Reads a string, represented in portable representation (varint length
// followed by the character array), up to `max_size` length.
//
// If the string length exceeds `max_size`, the result gets truncated to
// `max_size`, but the entire string gets skipped in the input stream.
template <typename InputIterator>
std::string ReadString(InputIterator& in, size_t max_size = SIZE_MAX) {
  uint64_t len = ReadVarU64(in);
  if (in.status() != kOk) return "";
  std::string result;
  if (len <= max_size) {
    // Common case.
    result.reserve(len);
    for (size_t i = 0; i < len; ++i) {
      char ch = (char)ReadU8(in);
      if (in.status() != kOk) return result;
      result.push_back(ch);
    }
    return result;
  }
  result.reserve(max_size);
  for (size_t i = 0; i < max_size; ++i) {
    char ch = (char)ReadU8(in);
    if (in.status() != kOk) return result;
    result.push_back(ch);
  }
  in.skip(len - max_size);
  return result;
}

// For memory iterators only. Reads a string, represented in portable
// representation (varint length followed by the character array), as
// a string_view up to `max_size` length, without copying the underlying data.
// (The returned string_view is backed by the underlying iterator's memory
// buffer).
//
// If the string length exceeds `max_size`, the result gets truncated to
// `max_size`, but the entire string gets skipped in the input stream.
template <typename InputIterator,
          typename std::enable_if<
              internal::MemoryIteratorTraits<InputIterator>::is_memory,
              bool>::type = true>
string_view ReadStringView(InputIterator& in, size_t max_size = SIZE_MAX) {
  uint64_t len = ReadVarU64(in);
  if (in.status() != kOk) return "";
  typename InputIterator::PtrType start = in.ptr();
  if (len <= max_size) {
    // Common case.
    in.skip(len);
    return string_view((const char*)start, in.ptr() - start);
  }
  in.skip(max_size);
  string_view result((const char*)start, in.ptr() - start);
  in.skip(len - max_size);
  return result;
}

}  // namespace roo_io
