#include "roo_io/reliable/packet_sender.h"

#include "roo_collections/hash.h"
#include "roo_io/memory/store.h"
#include "roo_io/third_party/nanocobs/cobs.h"
#include "roo_logging.h"

namespace roo_io {

PacketSender::PacketSender(OutputStream& out)
    : out_(out), buf_(new byte[256]) {}

void PacketSender::send(const roo::byte* buf, size_t len) {
  // We will use 4 bytes for checksum, and 2 bytes for COBS overhead.
  CHECK_LE(len, kMaxPacketSize);
  buf_[0] = COBS_TINYFRAME_SENTINEL_VALUE;
  memcpy(&buf_[1], buf, len);
  uint32_t hash = roo_collections::murmur3_32(&buf_[1], len, 0);
  roo_io::StoreBeU32(hash, &buf_[len + 1]);
  buf_[len + 5] = COBS_TINYFRAME_SENTINEL_VALUE;
  CHECK_EQ(COBS_RET_SUCCESS, cobs_encode_tinyframe(buf_.get(), len + 6));
  out_.writeFully(buf_.get(), len + 6);
  out_.flush();
}

}  // namespace roo_io