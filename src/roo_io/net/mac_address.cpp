#include "roo_io/net/mac_address.h"

#include <cstring>

#include "roo_io/text/string_printf.h"

namespace roo_io {

#if (defined UINT8_MAX)
MacAddress::MacAddress(unsigned char a0, unsigned char a1, unsigned char a2,
                       unsigned char a3, unsigned char a4, unsigned char a5) {
  addr_[0] = (roo_io::byte)a0;
  addr_[1] = (roo_io::byte)a1;
  addr_[2] = (roo_io::byte)a2;
  addr_[3] = (roo_io::byte)a3;
  addr_[4] = (roo_io::byte)a4;
  addr_[5] = (roo_io::byte)a5;
}
#endif

const MacAddress& MacAddress::Broadcast() {
  static MacAddress broadcast(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
  return broadcast;
}

bool operator==(const MacAddress& a, const MacAddress& b) {
  return memcmp(a.asBytes(), b.asBytes(), 6) == 0;
}

bool operator!=(const MacAddress& a, const MacAddress& b) {
  return memcmp(a.asBytes(), b.asBytes(), 6) != 0;
}

bool operator<(const MacAddress& a, const MacAddress& b) {
  return memcmp(a.asBytes(), b.asBytes(), 6) < 0;
}

bool operator>(const MacAddress& a, const MacAddress& b) {
  return memcmp(a.asBytes(), b.asBytes(), 6) > 0;
}

bool operator<=(const MacAddress& a, const MacAddress& b) {
  return memcmp(a.asBytes(), b.asBytes(), 6) <= 0;
}

bool operator>=(const MacAddress& a, const MacAddress& b) {
  return memcmp(a.asBytes(), b.asBytes(), 6) >= 0;
}

roo_logging::Stream& operator<<(roo_logging::Stream& s,
                                const MacAddress& addr) {
  const byte* data = addr.asBytes();
  s.printf("%02X-%02X-%02X-%02X-%02X-%02X", (int)data[0], (int)data[1],
           (int)data[2], (int)data[3], (int)data[4], (int)data[5]);
  return s;
}

std::string MacAddress::asString() const {
  return StringPrintf("%02X-%02X-%02X-%02X-%02X-%02X", (int)addr_[0],
                      (int)addr_[1], (int)addr_[2], (int)addr_[3],
                      (int)addr_[4], (int)addr_[5]);
}

void MacAddress::writeStringTo(char* buf) const {
  sprintf(buf, "%02X-%02X-%02X-%02X-%02X-%02X", (int)addr_[0], (int)addr_[1],
          (int)addr_[2], (int)addr_[3], (int)addr_[4], (int)addr_[5]);
}

uint64_t MacAddress::asU64() const {
  return (((uint64_t)addr_[0]) << 40) | (((uint64_t)addr_[1]) << 32) |
         (((uint64_t)addr_[2]) << 24) | (((uint64_t)addr_[3]) << 16) |
         (((uint64_t)addr_[4]) << 8) | (((uint64_t)addr_[5]) << 0);
}

bool MacAddress::assignFromU64(uint64_t addr) {
  if ((addr & 0xFFFF000000000000LL) != 0) return false;
  *this = MacAddress(addr >> 40, addr >> 32, addr >> 24, addr >> 16, addr >> 8,
                     addr >> 0);
  return true;
}

namespace {

bool parseHexDigit(char ch, uint8_t& result) {
  if (ch >= '0' && ch <= '9') {
    result = ch - '0';
    return true;
  }
  if (ch >= 'A' && ch <= 'F') {
    result = ch - 'A' + 10;
    return true;
  }
  if (ch >= 'a' && ch <= 'f') {
    result = ch - 'a' + 10;
    return true;
  }
  return false;
}

}  // namespace

bool MacAddress::parseFrom(const char* rep) {
  if (strlen(rep) < 17) return false;
  roo_io::byte raw[6];
  int pos = 0;
  while (pos < 6) {
    if (pos > 0) {
      if (*rep++ != '-') return false;
    }
    uint8_t hi, lo;
    if (!parseHexDigit(*rep++, hi)) return false;
    if (!parseHexDigit(*rep++, lo)) return false;
    raw[pos] = roo_io::byte{hi} << 4 | roo_io::byte{lo};
    ++pos;
  }
  *this = MacAddress(raw);
  return true;
}

void MacAddress::writeTo(byte* buf) const { memcpy(buf, addr_, 6); }

}  // namespace roo_io