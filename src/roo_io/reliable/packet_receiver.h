#pragma once

#include <memory>

#include "roo_io/core/input_stream.h"

namespace roo_io {

// Receives packets sent by PacketSenter. Implements data integrity (ensures
// data correctness) over a potentially unreliable underlying stream, such as
// UART/Serial.
//
// The data arrives in packets up to 250 bytes in size. The transport guarantees
// that receives packets have been transmitted correctly, although some packets
// may have gotten lost. If data corruption is detected in a packet, the entire
// packet is dropped.
//
// The underlying implementation uses 32-bit hashes to verify integrity, and
// uses COBS encoding to make sure that the receiver can recognize packet
// boundaries even in case of data loss or corruption.
class PacketReceiver {
 public:
  // Callback type to be called when a packet arrives.
  using ReceiverFn = std::function<void(const byte*, size_t)>;

  // Creates a packet receiver that reads data from the underlying input stream
  // (assumed unreliable), and invoking the specified callback `receiver_fn`
  // when a valid packet is received.
  //
  // The receiver_fn can be left unspecified, and supplied later by calling
  // `setReceiverFn`.
  PacketReceiver(InputStream& in, ReceiverFn receiver_fn = nullptr);

  // Must be called when there might be new data to read from the input
  // stream.
  void tryReceive();

  // Sets the new receiver callback, overwriting the previous one (if any).
  void setReceiverFn(ReceiverFn receiver_fn);

 private:
  void processPacket(byte* buf, size_t size);

  InputStream& in_;
  std::unique_ptr<byte[]> buf_;
  std::unique_ptr<byte[]> tmp_;
  size_t pos_;
  ReceiverFn receiver_fn_;
};

}  // namespace roo_io