#pragma once

#include <vector>

#include "roo_io/base/byte.h"
#include "roo_io/base/string_view.h"
#include "roo_io/data/read.h"
#include "roo_io/iterator/input_iterator.h"
#include "roo_io/third_party/u8c.h"

namespace roo_io {

class Utf8Decoder {
 public:
  // Creates a decoder that will represent the specified byte array as Unicode
  // code points.
  Utf8Decoder(const byte *data, size_t size) : ptr_(data), end_(data + size) {}

  // Creates a decoder that will represent the specified char array as Unicode
  // code points.
  Utf8Decoder(const char *data, size_t size)
      : Utf8Decoder((const byte *)data, size) {}

  // Creates a decoder that will represent the specified byte array as Unicode
  // code points.
  Utf8Decoder(const byte data[])
      : Utf8Decoder(data, sizeof(data) / sizeof(byte)) {}

  // Creates a decoder that will represent the specified byte array as Unicode
  // code points.
  Utf8Decoder(const char data[])
      : Utf8Decoder((const byte *)data, sizeof(data) / sizeof(byte)) {}

  // Convenience constructor that reads the input from a specifed string.
  Utf8Decoder(string_view s) : Utf8Decoder((const byte *)s.data(), s.size()) {}

#if __cplusplus >= 202002L
  // Convenience constructor that reads the input from a specifed string.
  Utf8Decoder(std::basic_string_view<char8_t> s)
      : Utf8Decoder((const byte *)s.data(), s.size()) {}
#endif

  const byte *data() const { return ptr_; }

  bool next(char32_t &result) {
    if (ptr_ == end_) return false;
    ptr_ += u8c::u8next_((const char *)ptr_, (const char *)end_, result);
    return true;
  }

 private:
  const byte *ptr_;
  const byte *end_;
};

template <typename OutputItr>
void DecodeUtfString(string_view s, OutputItr itr) {
  Utf8Decoder decoder(s);
  char32_t ch;
  while (decoder.next(ch)) *itr++ = ch;
}

inline std::vector<char32_t> DecodeUtfStringToVector(string_view s) {
  std::vector<char32_t> result;
  DecodeUtfString(s, std::back_inserter(result));
  return result;
}

// Writes a single Unicode code point, encoded as UTF-8, to the specified
// iterator.
template <typename OutputIterator>
void WriteUtf8Char(OutputIterator &itr, char32_t v) {
  if (v <= 0x7F) {
    itr.write((byte)v);
  } else if (v <= 0x7FF) {
    itr.write((byte)((v >> 6) | 0xC0));
    itr.write((byte)((v & 0x3F) | 0x80));
  } else if (v <= 0xFFFF) {
    itr.write((byte)((v >> 12) | 0xE0));
    itr.write((byte)(((v >> 6) & 0x3F) | 0x80));
    itr.write((byte)((v & 0x3F) | 0x80));
  } else {
    itr.write((byte)((v >> 18) | 0xF0));
    itr.write((byte)(((v >> 12) & 0x3F) | 0x80));
    itr.write((byte)(((v >> 6) & 0x3F) | 0x80));
    itr.write((byte)((v & 0x3F) | 0x80));
  }
}

// Writes the UTF-8 representation of the rune to buf. The `buf` must have
// sufficient size (4 is always safe). Returns the number of bytes actually
// written.
inline int WriteUtf8Char(byte *buf, char32_t ch) {
  if (ch <= 0x7F) {
    buf[0] = (byte)ch;
    return 1;
  }
  if (ch <= 0x7FF) {
    buf[1] = (byte)((ch & 0x3F) | 0x80);
    ch >>= 6;
    buf[0] = (byte)(ch | 0xC0);
    return 2;
  }
  if (ch <= 0xFFFF) {
    buf[2] = (byte)((ch & 0x3F) | 0x80);
    ch >>= 6;
    buf[1] = (byte)((ch & 0x3F) | 0x80);
    ch >>= 6;
    buf[0] = (byte)(ch | 0xE0);
    return 3;
  }
  buf[3] = (byte)((ch & 0x3F) | 0x80);
  ch >>= 6;
  buf[2] = (byte)((ch & 0x3F) | 0x80);
  ch >>= 6;
  buf[1] = (byte)((ch & 0x3F) | 0x80);
  ch >>= 6;
  buf[0] = (byte)(ch | 0xF0);
  return 4;
}

}  // namespace roo_io