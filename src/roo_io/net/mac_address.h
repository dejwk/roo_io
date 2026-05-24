#pragma once

#include <stdint.h>

#include <cstring>
#include <string>

#include "roo_io/base/byte.h"
#include "roo_logging.h"

namespace roo_io {

/// Stores and formats a 48-bit MAC address.
class MacAddress {
 public:
  /// Returns the broadcast MAC address `FF-FF-FF-FF-FF-FF`.
  static const MacAddress& Broadcast();

  /// Initializes the address to `00-00-00-00-00-00`.
  MacAddress() { memset(addr_, 0, 6); }

  /// Copies six bytes from `addr` into this address.
  MacAddress(const byte* addr) { memcpy(addr_, addr, 6); }

  /// Initializes the address from six individual octets.
  MacAddress(unsigned char a0, unsigned char a1, unsigned char a2,
             unsigned char a3, unsigned char a4, unsigned char a5);

#if (defined UINT8_MAX && __cplusplus >= 201703L)
  /// Copies six bytes from a `uint8_t` buffer into this address.
  MacAddress(const uint8_t* addr) : MacAddress((const byte*)addr) {}
#endif

  /// Returns the octet at `idx`.
  int operator[](int idx) const {
    DCHECK(idx >= 0 && idx <= 5);
    return (int)asBytes()[idx];
  }

  /// Returns the raw six-byte address.
  const byte* asBytes() const { return addr_; }

  /// Returns the address formatted as `XX-XX-XX-XX-XX-XX`.
  std::string asString() const;

  /// Returns the address packed into the low 48 bits of a 64-bit integer.
  uint64_t asU64() const;

  /// Writes the raw six-byte address into `buf`.
  void writeTo(byte* buf) const;

#if (defined UINT8_MAX && __cplusplus >= 201703L)
  /// Writes the raw six-byte address into a `uint8_t` buffer.
  void writeTo(uint8_t* buf) const { writeTo((byte*)buf); }
#endif

  /// Writes the address as `XX-XX-XX-XX-XX-XX` into `buf`.
  ///
  /// The caller must provide room for at least 18 characters including the
  /// terminating zero.
  void writeStringTo(char* buf) const;

  /// Assigns the address from the low 48 bits of `addr`.
  bool assignFromU64(uint64_t addr);

  /// Parses an address from `rep` formatted as hexadecimal octets.
  bool parseFrom(const char* rep);

 private:
  byte addr_[6];
};

/// Returns whether the two MAC addresses are identical.
bool operator==(const MacAddress& a, const MacAddress& b);
/// Returns whether the two MAC addresses differ.
bool operator!=(const MacAddress& a, const MacAddress& b);
/// Returns whether `a` sorts before `b` lexicographically by octet.
bool operator<(const MacAddress& a, const MacAddress& b);
/// Returns whether `a` sorts after `b` lexicographically by octet.
bool operator>(const MacAddress& a, const MacAddress& b);
/// Returns whether `a` sorts before or equals `b` lexicographically by octet.
bool operator<=(const MacAddress& a, const MacAddress& b);
/// Returns whether `a` sorts before or equals `b` lexicographically by octet.
bool operator<=(const MacAddress& a, const MacAddress& b);

/// Reads six bytes from `itr` and returns them as a MAC address.
template <typename InputIterator>
MacAddress ReadMacAddress(InputIterator& itr) {
  byte addr[] = {byte{0}, byte{0}, byte{0}, byte{0}, byte{0}, byte{0}};
  ReadBytes(itr, addr);
  return MacAddress(addr);
}

/// Writes the raw six-byte MAC address through `itr`.
template <typename OutputIterator>
void WriteMacAddress(OutputIterator& itr, const MacAddress& addr) {
  WriteBytes(itr, addr.asBytes(), 6);
}

/// Streams the printable MAC address representation into the logging sink.
roo_logging::Stream& operator<<(roo_logging::Stream& s, const MacAddress& addr);

}  // namespace roo_io

namespace std {

template <>
struct hash<roo_io::MacAddress> {
  /// Returns a hash derived from the packed 48-bit MAC address value.
  size_t operator()(const roo_io::MacAddress& addr) const {
    return std::hash<uint64_t>()(addr.asU64());
  }
};

}  // namespace std