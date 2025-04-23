#include "roo_io/reliable/streaming_retransmitter.h"

namespace roo_io {

namespace {

// Values are significant; must be 0-15, not change. They are used in the
// communication protocol.
//
// Packet formats:
//
// Generally, each packet consists of a 16-bit header, and some optional
// payload. The topmost 4 bits of the header (in network order) identify the
// packet type (i.e. one of the enum values, below). The remaining 12 bytes
// represent the packet's sequence number.
//
// * kDataPacket:
//   the payload is all application data.
//
// * kDataAckPacket:
//   Sent by the recipient, to confirm reception of all packets with sequence
//   numbers preceding the sequence number carried in the header (which we call
//   'unack_seq_number'). The (optional) payload consists of an arbitrary number
//   of bytes, which constitute the the lookahead 'ack' bitmap. That is,
//   successive bits, with most-significant bit first, indicate the 'ack' status
//   of packets with subsequent sequence numbers, starting with unack_seq_number
//   + 1. (Knowing that some packets have been received allows the sender to
//   avoid needless retransmissions).
//
// * kHandshakePacket:
//   Used in the handshake protocol. The payload consists of 9 bytes. The bytes
//   1-4 contain the sender's 32-bit stream ID (in the network order). The bytes
//   5-8 contain the acknowledgement of the previously received peer's 32-bit
//   stream ID (in the network order), or zero if we don't know it yet. The last
//   byte indicates whether the sender expects an acknowledgement of the
//   handshake (a non-zero value indicates that the ack is requested).
//   The handshake protocol is somewhat similar to TCP-IP connection protocol
//   (SYN/SYN-ACK/ACK): the initiating party sends a handshake message including
//   its randomly generated stream ID; the recipient responds with a similar
//   handshake message, acknowledging the sender's stream ID and including its
//   own, and the initiator acknowledges receipt by sending a final handshake
//   message.
//
// * kFlowControlPacket:
//   Sent by the recipient, to indicate maximum sequence number that the
//   recipient has space to receive.flow control packet: 12 bits of header is
//   the maximum number of packets that the receiver still has place to buffer;
//
enum PacketType {
  kDataPacket = 0,
  kDataAckPacket = 1,
  kHandshakePacket = 2,
  kFlowControlPacket = 3,
};

PacketType GetPacketType(uint16_t header) { return (PacketType)(header >> 12); }

uint16_t FormatPacketHeader(StreamingRetransmitter::SeqNum seq, PacketType type) {
  return (seq.raw() & 0x0FFF) | (type << 12);
}

roo_time::Interval Backoff(int retry_count) {
  float min_delay_us = 100.0f;     // 100us
  float max_delay_us = 100000.0f;  // 100ms
  float delay = pow(1.33, retry_count) * min_delay_us;
  if (delay > max_delay_us) {
    delay = max_delay_us;
  }
  // Randomize by +=20%, to make unrelated retries spread more evenly in time.
  delay += (float)delay * ((float)rand() / RAND_MAX - 0.5f) * 0.4f;
  return roo_time::Micros((uint64_t)delay);
}

}  // namespace

StreamingRetransmitter::StreamingRetransmitter(roo_io::PacketSender& sender,
                                               roo_io::PacketReceiver& receiver,
                                               unsigned int sendbuf_log2,
                                               unsigned int recvbuf_log2,
                                               ConnectionCb connection_cb)
    : sender_(sender),
      receiver_(receiver),
      out_buffers_(new OutBuffer[1 << sendbuf_log2]),
      current_out_buffer_(nullptr),
      out_ring_(sendbuf_log2, 0),
      next_to_send_(out_ring_.begin()),
      recv_himark_(out_ring_.begin() + (1 << sendbuf_log2)),
      in_buffers_(new InBuffer[1 << recvbuf_log2]),
      current_in_buffer_(nullptr),
      current_in_buffer_pos_(0),
      in_ring_(recvbuf_log2, 0),
      needs_ack_(false),
      unack_seq_(0),
      updated_recv_himark_(false),
      my_stream_id_(0),
      peer_stream_id_(0),
      sender_connected_(false),
      receiver_connected_(false),
      needs_handshake_ack_(false),
      successive_handshake_retries_(0),
      next_scheduled_handshake_update_(roo_time::Uptime::Start()),
      connection_cb_(std::move(connection_cb)),
      packets_sent_(0),
      packets_delivered_(0),
      packets_received_(0) {
  CHECK_LE(sendbuf_log2, 12);
  CHECK_LE(sendbuf_log2, recvbuf_log2);
  receiver.setReceiverFn(
      [this](const roo::byte* buf, size_t len) { packetReceived(buf, len); });
  while (my_stream_id_ == 0) my_stream_id_ = rand();
}

size_t StreamingRetransmitter::tryWrite(const roo::byte* buf, size_t count) {
  if (count == 0) return 0;
  size_t total_written = 0;
  do {
    CHECK_GE(recv_himark_, out_ring_.end());
    if (current_out_buffer_ == nullptr) {
      if (recv_himark_ == out_ring_.end()) {
        // No more tokens.
        break;
      }
      if (out_ring_.slotsFree() == 0) {
        break;
      }
      SeqNum pos = out_ring_.push();
      current_out_buffer_ = &getOutBuffer(pos);
      current_out_buffer_->reset(pos);
    }
    size_t written = current_out_buffer_->write(buf, count);
    total_written += written;
    buf += written;
    count -= written;
    if (current_out_buffer_->finished()) {
      // CHECK_GT(available_tokens_, 0);
      current_out_buffer_ = nullptr;
    }

  } while (count > 0);
  return total_written;
}

size_t StreamingRetransmitter::tryRead(roo::byte* buf, size_t count) {
  if (count == 0) return 0;
  size_t total_read = 0;
  do {
    if (current_in_buffer_ == nullptr) {
      if (in_ring_.empty()) break;
      current_in_buffer_ = &getInBuffer(in_ring_.begin());
      current_in_buffer_pos_ = 0;
    }
    if (current_in_buffer_->empty()) {
      // Not received yet.
      break;
    }
    CHECK_GT(current_in_buffer_->size(), current_in_buffer_pos_);
    size_t available = current_in_buffer_->size() - current_in_buffer_pos_;
    if (count < available) {
      memcpy(buf, current_in_buffer_->data() + current_in_buffer_pos_, count);
      total_read += count;
      current_in_buffer_pos_ += count;
      break;
    }
    memcpy(buf, current_in_buffer_->data() + current_in_buffer_pos_, available);
    buf += available;
    total_read += available;
    count -= available;
    current_in_buffer_->clear();
    current_in_buffer_ = nullptr;
    in_ring_.pop();
    updated_recv_himark_ = true;
  } while (count > 0);
  return total_read;
}

int StreamingRetransmitter::peek() {
  if (current_in_buffer_ == nullptr) {
    if (in_ring_.empty()) return -1;
    current_in_buffer_ = &getInBuffer(in_ring_.begin());
    current_in_buffer_pos_ = 0;
  }
  if (current_in_buffer_->empty()) {
    // Not received yet.
    return -1;
  }
  DCHECK_GT(current_in_buffer_->size(), current_in_buffer_pos_);
  return (int)current_in_buffer_->data()[current_in_buffer_pos_];
}

size_t StreamingRetransmitter::availableForRead() {
  if (current_in_buffer_ == nullptr) {
    if (in_ring_.empty()) return 0;
    current_in_buffer_ = &getInBuffer(in_ring_.begin());
    current_in_buffer_pos_ = 0;
  }
  if (current_in_buffer_->empty()) {
    // Not received yet.
    return 0;
  }
  DCHECK_GT(current_in_buffer_->size(), current_in_buffer_pos_);
  return current_in_buffer_->size() - current_in_buffer_pos_;
}

void StreamingRetransmitter::flush() {
  if (current_out_buffer_ != nullptr) {
    current_out_buffer_->flush();
  }
}

size_t StreamingRetransmitter::availableForWrite() {
  // In the extreme case, if flush is issued after every write, we might only
  // fit one byte per slot.
  return out_ring_.slotsFree();
}

void StreamingRetransmitter::OutBuffer::reset(SeqNum seq_id) {
  uint16_t header = FormatPacketHeader(seq_id, kDataPacket);
  roo_io::StoreBeU16(header, payload_);
  size_ = 0;
  acked_ = false;
  flushed_ = false;
  finished_ = false;
  expiration_ = roo_time::Uptime::Start();
  send_counter_ = 0;
}

void StreamingRetransmitter::reset() {
  receiver_connected_ = false;
  sender_connected_ = false;
  my_stream_id_ = 0;
  peer_stream_id_ = 0;
  // The stream ID is a random number, but it can't be zero.
  while (my_stream_id_ == 0) my_stream_id_ = rand();
  while (!out_ring_.empty()) {
    getOutBuffer(out_ring_.begin()).reset(out_ring_.begin());
    out_ring_.pop();
  }
  while (!in_ring_.empty()) {
    getInBuffer(in_ring_.begin()).clear();
    in_ring_.pop();
  }
  out_ring_.reset(0);
  in_ring_.reset(0);
  current_in_buffer_ = nullptr;
  current_in_buffer_pos_ = 0;
  needs_ack_ = false;
  updated_recv_himark_ = false;
  needs_handshake_ack_ = false;
  successive_handshake_retries_ = 0;
  current_out_buffer_ = nullptr;
  unack_seq_ = 0;
  recv_himark_ = out_ring_.begin() + out_ring_.capacity();

  next_scheduled_handshake_update_ = roo_time::Uptime::Start();
}

bool StreamingRetransmitter::loop() {
  conn();
  recv();
  ack();
  send();
  return true;
}

bool StreamingRetransmitter::conn() {
  if (!sender_connected_) {
    needs_handshake_ack_ = true;
    roo_time::Uptime now = roo_time::Uptime::Now();
    if (now < next_scheduled_handshake_update_) return false;
    next_scheduled_handshake_update_ =
        now + Backoff(successive_handshake_retries_++);
  }
  if (needs_handshake_ack_) {
    roo::byte buf[11];
    uint16_t header =
        FormatPacketHeader(out_ring_.begin(), kHandshakePacket);
    roo_io::StoreBeU16(header, buf);
    roo_io::StoreBeU32(my_stream_id_, buf + 2);
    roo_io::StoreBeU32(peer_stream_id_, buf + 6);
    roo_io::StoreU8(sender_connected_ ? 0x0 : 0xFF, buf + 10);
    sender_.send(buf, 11);
    needs_handshake_ack_ = false;
  }
  return true;
}

void StreamingRetransmitter::ack() {
  if (!receiver_connected_) return;
  // Send ack first, if needed.
  if (needs_ack_) {
    roo::byte buf[10];
    uint16_t payload = FormatPacketHeader(unack_seq_, kDataAckPacket);
    roo_io::StoreBeU16(payload, buf);

    // For now, we only send ack about up to 64 packets. This should be more
    // than enough in most cases. If needed, it can be extended, though; the
    // receiver will understand the arbitrary number of bytes in the bitmask,
    // not just 8.
    uint64_t ack_bitmask = 0;
    // Skipping the unack_seq_ itself, because it's status is obvious (unacked).
    SeqNum in_pos = unack_seq_ + 1;
    int idx = 63;
    while (idx >= 0 && in_ring_.contains(in_pos)) {
      if (!getInBuffer(in_pos).empty()) {
        ack_bitmask |= (((uint64_t)1) << idx);
      }
      ++in_pos;
      --idx;
    }
    if (ack_bitmask == 0) {
      sender_.send(buf, 2);
    } else {
      roo_io::StoreBeU64(ack_bitmask, buf + 2);
      int len = 10;
      // No need to send bytes that are all zero.
      while (buf[len - 1] == roo::byte{0}) --len;
      sender_.send(buf, len);
    }
    needs_ack_ = false;
  }
  if (updated_recv_himark_) {
    SeqNum recv_himark = in_ring_.begin() + in_ring_.capacity();
    roo::byte buf[2];
    uint16_t payload = FormatPacketHeader(recv_himark, kFlowControlPacket);
    roo_io::StoreBeU16(payload, buf);
    sender_.send(buf, 2);
    updated_recv_himark_ = false;
  }
}

bool StreamingRetransmitter::send() {
  if (!sender_connected_) return false;
  if (out_ring_.contains(next_to_send_)) {
    // Best-effort attempt to quickly send the next buffer in the sequence.
    OutBuffer& buf = getOutBuffer(next_to_send_);
    if (!buf.acked() && buf.flushed()) {
      if (!buf.finished()) buf.finish();
      if (buf.send_counter() == 0) {
        // Never sent before.
        sender_.send(buf.data(), buf.size());
        ++next_to_send_;
        ++packets_sent_;
        buf.markSent(roo_time::Uptime::Now());
        return true;
      }
    }
  }
  // Fall back: find the earliest finished buffer to send.
  roo_time::Uptime now = roo_time::Uptime::Now();
  SeqNum to_send = out_ring_.end();
  roo_time::Uptime min_send_time = roo_time::Uptime::Max();
  for (SeqNum pos = out_ring_.begin();
       pos < out_ring_.end() && pos < recv_himark_; ++pos) {
    OutBuffer& buf = getOutBuffer(pos);
    if (buf.acked()) {
      continue;
    }
    if (!buf.flushed()) {
      // No more ready to send buffers can follow.
      break;
    }
    if (!buf.finished() || buf.expiration() == roo_time::Uptime::Start()) {
      // This one can be sent immediately; no need to seek any further.
      to_send = pos;
      min_send_time = roo_time::Uptime::Start();
      break;
    }
    // This is a viable candidate.
    if (buf.expiration() < min_send_time) {
      to_send = pos;
      min_send_time = buf.expiration();
    }
  }
  if (!out_ring_.contains(to_send) || min_send_time > now) {
    return false;
  }

  OutBuffer& buf = getOutBuffer(to_send);
  if (!buf.finished()) {
    buf.finish();
  }
  sender_.send(buf.data(), buf.size());
  buf.markSent(now);
  next_to_send_ = to_send + 1;
  ++packets_sent_;
  return true;
}

void StreamingRetransmitter::packetReceived(const roo::byte* buf, size_t len) {
  uint16_t header = roo_io::LoadBeU16(buf);
  switch (GetPacketType(header)) {
    case kDataAckPacket: {
      if (!sender_connected_) return;
      // Remove all buffers up to the acked position.
      uint16_t truncated_seq_id = (header & 0x0FFF);
      SeqNum seq_id = out_ring_.restorePosHighBits(truncated_seq_id, 12);
      while (out_ring_.begin() < seq_id && !out_ring_.empty()) {
        getOutBuffer(out_ring_.begin()).reset(out_ring_.begin());
        out_ring_.pop();
        ++packets_delivered_;
      }
      size_t offset = 2;
      SeqNum out_pos = out_ring_.begin() + 1;
      SeqNum last_acked = out_ring_.begin() - 1;
      while (offset < len) {
        uint8_t val = (uint8_t)buf[offset];
        for (int i = 7; i >= 0; --i) {
          if (out_ring_.contains(out_pos) && (val & (1 << i)) != 0) {
            getOutBuffer(out_pos).ack();
            last_acked = out_pos;
          }
          out_pos++;
        }
        offset++;
      }
      // Try to increase send throughput by quickly detecting dropped packets,
      // interpreting skip-ack as nack for packets that have only been sent once
      // (which means that, assuming in-order delivery of the underlying package
      // writer, if they were to be delivered, they would have been already
      // delivered).
      if (out_ring_.contains(last_acked)) {
        bool next_to_send_updated = false;
        // Rush re-delivery of any packets that have only been sent once and
        // nacked.
        for (SeqNum pos = out_ring_.begin(); pos != last_acked; ++pos) {
          auto& buf = getOutBuffer(pos);
          if (!buf.acked() && buf.send_counter() == 1) {
            buf.rush();
            // Also, send the first nacked packet ASAP, to unblock the reader.
            if (!next_to_send_updated) {
              next_to_send_updated = true;
              next_to_send_ = pos;
            }
          }
        }
      }
      return;
    }
    case kFlowControlPacket: {
      if (!sender_connected_) return;
      // Update to available slots received.
      uint16_t recv_himark = (header & 0x0FFF);
      recv_himark_ = out_ring_.restorePosHighBits(recv_himark, 12);
      return;
    }
    case kHandshakePacket: {
      if (len != 11) {
        // Malformed packet.
        return;
      }
      uint16_t peer_seq_num = header & 0x0FFF;
      uint32_t peer_stream_id = roo_io::LoadBeU32(buf + 2);
      uint32_t ack_stream_id = roo_io::LoadBeU32(buf + 6);
      uint8_t want_ack = roo_io::LoadU8(buf + 10);
      if (!receiver_connected_ || peer_stream_id != peer_stream_id_) {
        // Interpret as a new request.
        if (receiver_connected_) {
          receiver_connected_ = false;
          while (!out_ring_.empty()) {
            SeqNum pos = out_ring_.begin();
            getOutBuffer(pos).reset(pos);
            out_ring_.pop();
          }
          out_ring_.reset(peer_seq_num);
          recv_himark_ = out_ring_.begin() + out_ring_.capacity();
        }
        if (!in_ring_.empty()) {
          return;
        }
        if (peer_stream_id_ != 0 && connection_cb_ != nullptr) {
          connection_cb_();
        }
        in_ring_.reset(peer_seq_num);
        receiver_connected_ = true;
        peer_stream_id_ = peer_stream_id;
        unack_seq_ = peer_seq_num;
      } else {
        // Need to re-ack.
      }
      if (ack_stream_id == my_stream_id_) {
        sender_connected_ = true;
      }
      needs_handshake_ack_ = (want_ack != 0);
      return;
    }
    case kDataPacket: {
      if (!receiver_connected_) {
        return;
      }
      uint16_t truncated_seq_id = (header & 0x0FFF);
      SeqNum seq_id = in_ring_.restorePosHighBits(truncated_seq_id, 12);
      if (!in_ring_.contains(seq_id)) {
        if (seq_id < in_ring_.begin()) {
          // Retransmit of a package that we have acked before. (Maybe the ack
          // was lost.) Ignoring, but re-triggering the ack.
          needs_ack_ = true;
          return;
        }
        // See if we can extend the buf to add the new packet.
        size_t advance = seq_id - in_ring_.end() + 1;
        if (advance > in_ring_.slotsFree()) {
          return;
        }
        for (size_t i = 0; i < advance; ++i) {
          getInBuffer(in_ring_.push()).clear();
        }
        updated_recv_himark_ = true;
        DCHECK(in_ring_.contains(seq_id))
            << seq_id << ", " << in_ring_.begin() << "--"
            << in_ring_.end();
      }
      InBuffer& buffer = getInBuffer(seq_id);
      if (!buffer.empty()) {
        return;
      }
      buffer.set(buf + 2, len - 2);
      // Note: we send ack even if the packet we just received wasn't the oldest
      // unacked (i.e. even if we don't update unack_seq_), because we are
      // sending skip-acks as well.
      needs_ack_ = true;
      if (seq_id == unack_seq_) {
        // Update the unack seq.
        do {
          ++unack_seq_;
          ++packets_received_;
        } while (in_ring_.contains(unack_seq_) &&
                 !getInBuffer(unack_seq_).empty());
      }
      return;
    }
    default: {
      // Unrecognized packet type; ignoreing.
    }
  }
}

}  // namespace roo_io
