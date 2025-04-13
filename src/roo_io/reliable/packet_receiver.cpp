#include "roo_io/reliable/packet_receiver.h"

#include "roo_collections/hash.h"
#include "roo_io/memory/load.h"
#include "roo_io/third_party/nanocobs/cobs.h"
#include "roo_logging.h"

namespace roo_io {

PacketReceiver::PacketReceiver(InputStream& in, ReceiverFn receiver_fn)
    : in_(in),
      buf_(new byte[256]),
      tmp_(new byte[256]),
      pos_(0),
      receiver_fn_(std::move(receiver_fn)) {}

void PacketReceiver::setReceiverFn(ReceiverFn receiver_fn) {
  receiver_fn_ = std::move(receiver_fn);
}

void PacketReceiver::tryReceive() {
  while (true) {
    size_t len = in_.tryRead(tmp_.get(), 256);
    if (len == 0) return;
    byte* data = &tmp_[0];
    while (len > 0) {
      // Find the possible packet delimiter (zero byte).
      const byte* delim = std::find(data, data + len, byte{0});
      size_t increment = delim - data;
      bool finished = (increment < len);
      if (finished) {
        ++increment;
        if (pos_ + increment <= 256) {
          if (pos_ == 0) {
            processPacket(data, increment);
          } else {
            memcpy(&buf_[pos_], data, increment);
            processPacket(buf_.get(), pos_ + increment);
          }
        }
        pos_ = 0;
      } else {
        if (pos_ + increment < 256) {
          memcpy(&buf_[pos_], data, increment);
          pos_ += increment;
        } else {
          pos_ = 256;
        }
      }
      data += increment;
      len -= increment;

      // Alternative implementation:
      // if (pos_ + increment < 256) {
      //   // Fits within the 'max packet' size.
      //   memcpy(&buf_[pos_], data, increment);
      //   if (finished) {
      //     buf_[pos_ + increment] = 0;
      //     processPacket(pos_ + increment + 1);
      //     pos_ = 0;
      //     // Skip the zero byte itself.
      //     increment++;
      //   } else {
      //     pos_ += increment;
      //   }
      // } else {
      //   // Ignore all bytes up to the next packet.
      //   if (finished) {
      //     pos_ = 0;
      //     increment++;
      //   } else {
      //     pos_ = 256;
      //   }
      // }
      // data += increment;
      // len -= increment;
    }
  }
}

void PacketReceiver::processPacket(byte* buf, size_t size) {
  if (cobs_decode_tinyframe(buf, size) == COBS_RET_SUCCESS) {
    // Verify the checksum.
    uint32_t computed_hash = roo_collections::murmur3_32(&buf[1], size - 6, 0);
    uint32_t received_hash = roo_io::LoadBeU32(&buf[size - 5]);
    if (computed_hash != received_hash) {
      // Invalid checksum. Dropping packet.
      return;
    }
    if (receiver_fn_ != nullptr) receiver_fn_(&buf[1], size - 6);
  } else {
    // Invalid payload (COBS decoding failed). Dropping packet.
  }
}

}  // namespace roo_io