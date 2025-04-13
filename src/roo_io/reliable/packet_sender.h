#pragma once

#include <memory>

#include "roo_io/core/output_stream.h"

namespace roo_io {

// Implements data integrity (ensures data correctness) over a potentially
// unreliable underlying stream, such as UART/Serial.
//
// The caller needs to provide data in packets, up to kMaxPacketSize (250
// bytes). The transport guarantees that packets are transmitted correctly or
// not at all. That is, if data corruption is detected in a packet, the entire
// packet is dropped.
//
// The underlying implementation uses 32-bit hashes to verify integrity, and
// uses COBS encoding to make sure that the receiver can recognize packet
// boundaries even in case of data loss or corruption.
class PacketSender {
 public:
  // Maximum size of the packet that can be sent.
  constexpr static int kMaxPacketSize = 250;

  // Creates the sender that will write packets to the underlying output stream
  // (which is assumed to be possibly unreliable, e.g. possibly dropping,
  // confusing, or reordering data.)
  PacketSender(OutputStream& out);

  // Sends the specified data packet.
  void send(const roo::byte* buf, size_t len);

 private:
  OutputStream& out_;
  // Work buffer, allocated in the constructor.
  std::unique_ptr<byte[]> buf_;
};

}  // namespace roo_io