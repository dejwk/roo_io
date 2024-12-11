#pragma once

#include "roo_io/base/byte.h"
#include "roo_io/base/string_view.h"

namespace roo_io {

// Writes the UTF-8 representation of the rune to buf. The `buf` must have
// sufficient size (4 is always safe). Returns the number of bytes actually
// written.
inline int Utf8Encode(char32_t rune, byte *buf) {
  if (rune <= 0x7F) {
    buf[0] = (byte)rune;
    return 1;
  }
  if (rune <= 0x7FF) {
    buf[1] = (byte)((rune & 0x3F) | 0x80);
    rune >>= 6;
    buf[0] = (byte)(rune | 0xC0);
    return 2;
  }
  if (rune <= 0xFFFF) {
    buf[2] = (byte)((rune & 0x3F) | 0x80);
    rune >>= 6;
    buf[1] = (byte)((rune & 0x3F) | 0x80);
    rune >>= 6;
    buf[0] = (byte)(rune | 0xE0);
    return 3;
  }
  buf[3] = (byte)((rune & 0x3F) | 0x80);
  rune >>= 6;
  buf[2] = (byte)((rune & 0x3F) | 0x80);
  rune >>= 6;
  buf[1] = (byte)((rune & 0x3F) | 0x80);
  rune >>= 6;
  buf[0] = (byte)(rune | 0xF0);
  return 4;
}

// Utility that can iteratively retrieve subsequent decoded Unicode code points,
// out of UTF8-encoded input.
//
// Note: in the future, may be reimplemented as a decorator over a byte
// iterator, but benchmarks are needed to confirm that it doesn't slow down the
// main usage scenario (which is to iterate over bytes already read into
// memory, e.g. a string).
//
// Note: as of now, this decoder does not reject malformed UTF; it simply
// ignores incorrect bits in some cases. This will be changed in the future
// versions.
class Utf8Decoder {
 public:
  // Creates a decoder that will represent the specified byte array as Unicode
  // code points.
  Utf8Decoder(const byte *data, size_t size) : data_(data), remaining_(size) {}

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

  bool has_next() const { return remaining_ > 0; }

  const byte *data() const { return data_; }

  size_t remaining() const { return remaining_; }

  char32_t next() {
    --remaining_;
    byte first = *data_++;
    // 7 bit Unicode.
    if ((first & byte{0x80}) == byte{0x00}) {
      return (char32_t)first;
    }

    // 11 bit Unicode.
    if (((first & byte{0xE0}) == byte{0xC0}) && (remaining_ >= 1)) {
      --remaining_;
      byte second = *data_++;
      return (((char32_t)first & 0x1F) << 6) | ((char32_t)second & 0x3F);
    }

    // 16 bit Unicode.
    if (((first & byte{0xF0}) == byte{0xE0}) && (remaining_ >= 2)) {
      remaining_ -= 2;
      byte second = *data_++;
      byte third = *data_++;
      return (((char32_t)first & 0x0F) << 12) |
             (((char32_t)second & 0x3F) << 6) | (((char32_t)third & 0x3F));
    }

    // 21 bit Unicode not supported so fall-back to extended ASCII
    if ((first & byte{0xF8}) == byte{0xF0} && (remaining_ >= 3)) {
      remaining_ -= 3;
      byte second = *data_++;
      byte third = *data_++;
      byte fourth = *data_++;
      return (((char32_t)first & 0x07) << 18) |
             (((char32_t)second & 0x3F) << 12) |
             (((char32_t)third & 0x3F) << 6) | (((char32_t)fourth & 0x3F));
    }
    return char32_t{0xFFFD};
  }

 private:
  const byte *data_;
  size_t remaining_;
};

}  // namespace roo_io