#pragma once

#include <vector>

#include "roo_backport.h"
#include "roo_backport/string_view.h"
#include "roo_io/base/byte.h"
#include "roo_io/core/input_iterator.h"
#include "roo_io/data/read.h"
#include "roo_io/third_party/u8c.h"

namespace roo_io {

/// Iterates over a UTF-8 byte sequence and decodes it into Unicode code points.
class Utf8Decoder {
 public:
  /// Creates a decoder over the specified UTF-8 byte sequence.
  Utf8Decoder(const byte *data, size_t size) : ptr_(data), end_(data + size) {}

  /// Creates a decoder over the specified UTF-8 character sequence.
  Utf8Decoder(const char *data, size_t size)
      : Utf8Decoder((const byte *)data, size) {}

  /// Creates a decoder over a fixed-size UTF-8 byte array.
  template <size_t N>
  Utf8Decoder(const byte data[N]) : Utf8Decoder(data, N) {}

  /// Creates a decoder over a fixed-size UTF-8 character array.
  template <size_t N>
  Utf8Decoder(const char data[N]) : Utf8Decoder((const byte *)data, N) {}

  /// Creates a decoder over the contents of the specified string view.
  Utf8Decoder(roo::string_view s)
      : Utf8Decoder((const byte *)s.data(), s.size()) {}

#if __cplusplus >= 202002L
  /// Creates a decoder over the contents of the specified UTF-8 string view.
  Utf8Decoder(std::basic_string_view<char8_t> s)
      : Utf8Decoder((const byte *)s.data(), s.size()) {}
#endif

  /// Returns the next unread byte of the underlying UTF-8 sequence.
  const byte *data() const { return ptr_; }

  /// Decodes the next Unicode code point into `result`.
  ///
  /// Returns `false` when the decoder has reached the end of the input.
  bool next(char32_t &result) {
    if (ptr_ == end_) return false;
    ptr_ += u8c::u8next_((const char *)ptr_, (const char *)end_, result);
    return true;
  }

 private:
  const byte *ptr_;
  const byte *end_;
};

/// Decodes the UTF-8 contents of `s` and appends code points to `itr`.
template <typename OutputItr>
void DecodeUtfString(roo::string_view s, OutputItr itr) {
  Utf8Decoder decoder(s);
  char32_t ch;
  while (decoder.next(ch)) *itr++ = ch;
}

/// Decodes the UTF-8 contents of `s` into a freshly allocated vector.
inline std::vector<char32_t> DecodeUtfStringToVector(roo::string_view s) {
  std::vector<char32_t> result;
  DecodeUtfString(s, std::back_inserter(result));
  return result;
}

/// Encodes one Unicode code point as UTF-8 and writes it through `itr`.
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

/// Encodes one Unicode code point as UTF-8 into `buf`.
///
/// The caller must provide enough storage for the result; four bytes are
/// always sufficient. Returns the number of bytes written.
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

/// Encodes one Unicode code point as UTF-8 into a character buffer.
inline int WriteUtf8Char(char *buf, char32_t ch) {
  return WriteUtf8Char((byte *)buf, ch);
}

}  // namespace roo_io