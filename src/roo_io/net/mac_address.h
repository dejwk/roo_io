#pragma once

#include <stdint.h>

#include <cstring>
#include <string>

#include "roo_io/base/byte.h"
#include "roo_logging.h"

namespace roo_io {

class MacAddress {
 public:
  static const MacAddress& Broadcast();

  MacAddress() { memset(addr_, 0, 6); }

  MacAddress(const byte* addr) { memcpy(addr_, addr, 6); }

  MacAddress(unsigned char a0, unsigned char a1, unsigned char a2,
             unsigned char a3, unsigned char a4, unsigned char a5);

#if (defined UINT8_MAX && __cplusplus >= 201703L)
  MacAddress(const uint8_t* addr) : MacAddress((const byte*)addr) {}
#endif

  int operator[](int idx) const {
    DCHECK(idx >= 0 && idx <= 5);
    return (int)asBytes()[idx];
  }

  const byte* asBytes() const { return addr_; }

  std::string asString() const;

  uint64_t asU64() const;

  void writeTo(byte* buf) const;

#if (defined UINT8_MAX && __cplusplus >= 201703L)
  void writeTo(uint8_t* buf) const { writeTo((byte*)buf); }
#endif

  static MacAddress FromU64(uint64_t addr);

 private:
  byte addr_[6];
};

bool operator==(const MacAddress& a, const MacAddress& b);
bool operator!=(const MacAddress& a, const MacAddress& b);
bool operator<(const MacAddress& a, const MacAddress& b);
bool operator>(const MacAddress& a, const MacAddress& b);
bool operator<=(const MacAddress& a, const MacAddress& b);
bool operator<=(const MacAddress& a, const MacAddress& b);

template <typename InputIterator>
MacAddress ReadMacAddress(InputIterator& itr) {
  byte addr[] = {byte{0}, byte{0}, byte{0}, byte{0}, byte{0}, byte{0}};
  ReadBytes(itr, addr);
  return MacAddress(addr);
}

template <typename OutputIterator>
void WriteMacAddress(OutputIterator& itr, const MacAddress& addr) {
  WriteBytes(itr, addr.asBytes(), 6);
}

roo_logging::Stream& operator<<(roo_logging::Stream& s, const MacAddress& addr);

}  // namespace roo_io

namespace std {

template<>
struct hash<roo_io::MacAddress> {
  size_t operator()(const roo_io::MacAddress& addr) const {
    return std::hash<uint64_t>()(addr.asU64());
  }
};

}