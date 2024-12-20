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
  s.printf("%02X:%02X:%02X:%02X:%02X:%02X", (int)data[0], (int)data[1],
           (int)data[2], (int)data[3], (int)data[4], (int)data[5]);
  return s;
}

std::string MacAddress::asString() const {
  return StringPrintf("%02X:%02X:%02X:%02X:%02X:%02X", (int)addr_[0],
                      (int)addr_[1], (int)addr_[2], (int)addr_[3],
                      (int)addr_[4], (int)addr_[5]);
}

uint64_t MacAddress::asU64() const {
  return (((uint64_t)addr_[0]) << 40) | (((uint64_t)addr_[1]) << 32) |
         (((uint64_t)addr_[2]) << 24) | (((uint64_t)addr_[3]) << 16) |
         (((uint64_t)addr_[4]) << 8) | (((uint64_t)addr_[5]) << 0);
}

MacAddress MacAddress::FromU64(uint64_t addr) {
  return MacAddress(addr >> 40, addr >> 32, addr >> 24, addr >> 16, addr >> 8,
                    addr >> 0);
}

void MacAddress::writeTo(byte* buf) const { memcpy(buf, addr_, 6); }

}  // namespace roo_io