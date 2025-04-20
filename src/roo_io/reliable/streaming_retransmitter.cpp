#include "roo_io/reliable/streaming_retransmitter.h"

namespace roo_io {

namespace {

// Values are significant; must be 0-15, not change. They are used in the
// communication protocol.
//
// Packet formats:
// * data packet: 12 bits of header is the packet seq num; the rest is payload.
// * ack packet: 12 bits of header is seq num just after all that the receiver
//   has already seen;
// * flow control packet: 12 bits of header is the maximum number of packets
//   that the receiver still has place to buffer;
// * connect packet:
//   - 12 bits of header is the first seq num that the sender is going to use,
//   - if two next bytes are present, they contain the ack of the connect seq
//     number sent by the peer.
//
// Handshake protocol:
// * a peer does not send anything until it sends a handshake packet and
//   receives an ack to it;
// * a peer does not accept anything until it first received a handshake packet.
enum PacketType {
  kDataPacket = 0,
  kDataAckPacket = 1,
  kHandshakePacket = 2,
  kFlowControlPacket = 3,
};
PacketType GetPacketType(uint16_t header) { return (PacketType)(header >> 12); }

uint16_t FormatPacketHeader(uint16_t in, PacketType type) {
  return in | (type << 12);
}

roo_time::Interval Backoff(int retry_count) {
  float min_delay_us = 10.0f;      // 10us
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
      sendbuf_capacity_(1 << sendbuf_log2),
      out_buffers_(new OutBuffer[1 << sendbuf_log2]),
      current_out_buffer_(nullptr),
      out_ring_(sendbuf_log2, 0),
      next_to_send_(out_ring_.start_pos()),
      available_tokens_(sendbuf_capacity_),
      recvbuf_capacity_(1 << recvbuf_log2),
      in_buffers_(new InBuffer[1 << recvbuf_log2]),
      current_in_buffer_(nullptr),
      current_in_buffer_pos_(0),
      in_ring_(recvbuf_log2, 0),
      needs_ack_(false),
      unack_seq_(0),
      needs_token_send_(false),
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
    if (current_out_buffer_ == nullptr) {
      if (available_tokens_ == 0) {
        // No more tokens.
        break;
      }
      if (out_ring_.slotsFree() == 0) {
        break;
      }
      uint32_t pos = out_ring_.push();
      current_out_buffer_ = &(out_buffers_.get())[out_ring_.offset_for(pos)];
      current_out_buffer_->reset(pos);
    }
    size_t written = current_out_buffer_->write(buf, count);
    total_written += written;
    buf += written;
    count -= written;
    if (current_out_buffer_->finished()) {
      --available_tokens_;
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
      current_in_buffer_ =
          &(in_buffers_.get())[in_ring_.offset_for(in_ring_.start_pos())];
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
    needs_token_send_ = true;
  } while (count > 0);
  return total_read;
}

int StreamingRetransmitter::peek() {
  if (current_in_buffer_ == nullptr) {
    if (in_ring_.empty()) return -1;
    current_in_buffer_ =
        &(in_buffers_.get())[in_ring_.offset_for(in_ring_.start_pos())];
    current_in_buffer_pos_ = 0;
  }
  if (current_in_buffer_->empty()) {
    // Not received yet.
    return -1;
  }
  CHECK_GT(current_in_buffer_->size(), current_in_buffer_pos_);
  return (int)current_in_buffer_->data()[current_in_buffer_pos_];
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

void StreamingRetransmitter::OutBuffer::reset(uint32_t seq_id) {
  uint16_t header = FormatPacketHeader(seq_id & 0x0FFF, kDataPacket);
  roo_io::StoreBeU16(header, payload_);
  size_ = 0;
  acked_ = false;
  flushed_ = false;
  finished_ = false;
}

void StreamingRetransmitter::reset() {
  receiver_connected_ = false;
  sender_connected_ = false;
  my_stream_id_ = 0;
  peer_stream_id_ = 0;
  while (my_stream_id_ == 0) my_stream_id_ = rand();
}

bool StreamingRetransmitter::sendLoop() {
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
        FormatPacketHeader(out_ring_.start_pos() & 0x0FFF, kHandshakePacket);
    roo_io::StoreBeU16(header, buf);
    roo_io::StoreBeU32(my_stream_id_, buf + 2);
    roo_io::StoreBeU32(peer_stream_id_, buf + 6);
    roo_io::StoreU8(sender_connected_ ? 0x0 : 0xFF, buf + 10);
    sender_.send(buf, 11);
    needs_handshake_ack_ = false;
  }
  if (receiver_connected_) {
    // Send ack first, if needed.
    if (needs_ack_) {
      roo::byte buf[2];
      uint16_t payload =
          FormatPacketHeader(unack_seq_ & 0x0FFF, kDataAckPacket);
      roo_io::StoreBeU16(payload, buf);
      sender_.send(buf, 2);
      needs_ack_ = false;
    }
    if (needs_token_send_) {
      roo::byte buf[2];
      uint16_t payload =
          FormatPacketHeader(in_ring_.slotsFree() & 0x0FFF, kFlowControlPacket);
      roo_io::StoreBeU16(payload, buf);
      sender_.send(buf, 2);
      needs_token_send_ = false;
    }
  }
  if (sender_connected_) {
    if (out_ring_.empty()) {
      // Nothing to send.
      return false;
    }
    // Skip all acked;
    if (!out_ring_.contains(next_to_send_)) {
      next_to_send_ = out_ring_.start_pos();
      return true;
    }
    while (out_ring_.contains(next_to_send_)) {
      OutBuffer& buf = getOutBuffer(next_to_send_);
      if (buf.acked()) {
        ++next_to_send_;
        continue;
      }
      if (!buf.flushed()) {
        return false;
      }
      // Found an outgoing packet to send.
      if (!buf.finished()) {
        buf.finish();
        --available_tokens_;
      }
      sender_.send(buf.data(), buf.size());
      ++next_to_send_;
      ++packets_sent_;
      return true;
    }
  }
  return false;
}

void StreamingRetransmitter::packetReceived(const roo::byte* buf, size_t len) {
  uint16_t header = roo_io::LoadBeU16(buf);
  switch (GetPacketType(header)) {
    case kDataAckPacket: {
      if (!sender_connected_) return;
      // Remove all buffers up to the acked position.
      uint16_t truncated_seq_id = (header & 0x0FFF);
      uint32_t seq_id = out_ring_.restorePosHighBits(truncated_seq_id, 12);
      while ((int32_t)(seq_id - out_ring_.start_pos()) > 0 &&
             !out_ring_.empty()) {
        out_ring_.pop();
        ++packets_delivered_;
      }
      return;
    }
    case kFlowControlPacket: {
      // Update to available slots received.
      uint16_t tokens = (header & 0x0FFF);
      available_tokens_ = tokens;
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
        }
        if (!in_ring_.empty()) {
          return;
        }
        if (peer_stream_id_ != 0 && connection_cb_ != nullptr) {
          connection_cb_();
        }
        in_ring_.reset(header & 0x0FFF);
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
      uint16_t truncated_seq_id = (header & 0x0FFF);
      uint32_t seq_id = in_ring_.restorePosHighBits(truncated_seq_id, 12);
      if (!in_ring_.contains(seq_id)) {
        if ((int32_t)(seq_id - in_ring_.start_pos()) < 0) {
          // Retransmit of a package that we have acked before. (Maybe the ack
          // was lost.) Ignoring, but re-triggering the ack.
          needs_ack_ = true;
          return;
        }
        // See if we can extend the buf to add the new packet.
        size_t advance = seq_id - in_ring_.end_pos() + 1;
        if (advance > in_ring_.slotsFree()) {
          return;
        }
        in_ring_.push(advance);
        needs_token_send_ = true;
        CHECK(in_ring_.contains(seq_id))
            << seq_id << ", " << in_ring_.start_pos() << "--"
            << in_ring_.end_pos();
      }
      InBuffer& buffer = getInBuffer(seq_id);
      if (!buffer.empty()) {
        return;
      }
      buffer.set(buf + 2, len - 2);
      if (seq_id == unack_seq_) {
        // Update the unack seq.
        needs_ack_ = true;
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
