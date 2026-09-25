#include "roo_io/text/unicode.h"

namespace roo_io {

bool IsValidUtf8(roo::string_view text) {
  const uint8_t* data = reinterpret_cast<const uint8_t*>(text.data());
  size_t index = 0;
  while (index < text.size()) {
    uint8_t first = data[index++];
    if (first <= 0x7F) continue;
    if (first >= 0xC2 && first <= 0xDF) {
      if (index >= text.size() || data[index] < 0x80 || data[index] > 0xBF) {
        return false;
      }
      ++index;
      continue;
    }
    if (first >= 0xE0 && first <= 0xEF) {
      if (index + 1 >= text.size()) return false;
      uint8_t second = data[index];
      uint8_t third = data[index + 1];
      if (third < 0x80 || third > 0xBF ||
          (first == 0xE0 && (second < 0xA0 || second > 0xBF)) ||
          (first == 0xED && (second < 0x80 || second > 0x9F)) ||
          ((first != 0xE0 && first != 0xED) &&
           (second < 0x80 || second > 0xBF))) {
        return false;
      }
      index += 2;
      continue;
    }
    if (first >= 0xF0 && first <= 0xF4) {
      if (index + 2 >= text.size()) return false;
      uint8_t second = data[index];
      uint8_t third = data[index + 1];
      uint8_t fourth = data[index + 2];
      if (third < 0x80 || third > 0xBF || fourth < 0x80 || fourth > 0xBF ||
          (first == 0xF0 && (second < 0x90 || second > 0xBF)) ||
          (first == 0xF4 && (second < 0x80 || second > 0x8F)) ||
          ((first != 0xF0 && first != 0xF4) &&
           (second < 0x80 || second > 0xBF))) {
        return false;
      }
      index += 3;
      continue;
    }
    return false;
  }
  return true;
}

}  // namespace roo_io
