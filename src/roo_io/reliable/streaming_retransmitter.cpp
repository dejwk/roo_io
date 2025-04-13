#include "roo_io/reliable/streaming_retransmitter.h"

namespace roo_io {

StreamingRetransmitter::StreamingRetransmitter(roo_io::PacketSender& sender,
                                               roo_io::PacketReceiver& receiver,
                                               unsigned int sendbuf_log2,
                                               unsigned int recvbuf_log2)
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
      needs_token_send_(false) {
  CHECK_LE(sendbuf_log2, 12);
  CHECK_LE(sendbuf_log2, recvbuf_log2);
  receiver.setReceiverFn(
      [this](const roo::byte* buf, size_t len) { packetReceived(buf, len); });
}

size_t StreamingRetransmitter::tryWrite(const roo::byte* buf, size_t count) {
  if (count == 0) return 0;
  size_t total_written = 0;
  do {
    if (current_out_buffer_ == nullptr) {
      if (available_tokens_ == 0) {
        // No more tokens.
        return 0;
      }
      if (out_ring_.slotsFree() == 0) {
        return 0;
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
  return current_in_buffer_->data()[current_in_buffer_pos_];
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

bool StreamingRetransmitter::sendLoop() {
  // Send ack first, if needed.
  if (needs_ack_) {
    roo::byte buf[2];
    uint16_t payload = (unack_seq_ & 0x0FFF) | 0x1000;
    roo_io::StoreBeU16(payload, buf);
    sender_.send(buf, 2);
    needs_ack_ = false;
  }
  if (needs_token_send_) {
    roo::byte buf[2];
    uint16_t payload = (in_ring_.slotsFree() & 0x0FFF) | 0x2000;
    roo_io::StoreBeU16(payload, buf);
    sender_.send(buf, 2);
    needs_token_send_ = false;
  }
  if (out_ring_.empty()) {
    // Nothing to send.
    return false;
  }
  // Skip all acked;
  if (!out_ring_.contains(next_to_send_)) {
    next_to_send_ = out_ring_.start_pos();
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
    return true;
  }
  return false;
}

void StreamingRetransmitter::packetReceived(const roo::byte* buf, size_t len) {
  uint16_t header = roo_io::LoadBeU16(buf);
  if ((header & 0xF000) == 0x1000) {
    // Ack received. Remove all buffers up to the acked position.
    uint16_t truncated_seq_id = (header & 0x0FFF);
    uint32_t seq_id = out_ring_.restorePosHighBits(truncated_seq_id, 12);
    while ((int32_t)(seq_id - out_ring_.start_pos()) > 0) {
      out_ring_.pop();
    }
    return;
  }
  if ((header & 0xF000) == 0x2000) {
    // Update to available slots received.
    uint16_t tokens = (header & 0x0FFF);
    available_tokens_ = tokens;
    return;
  }
  if ((header & 0xF000) != 0) {
  }
  uint16_t truncated_seq_id = (header & 0x0FFF);
  uint32_t seq_id = in_ring_.restorePosHighBits(truncated_seq_id, 12);
  if (!in_ring_.contains(seq_id)) {
    if ((int32_t)(seq_id - in_ring_.start_pos()) < 0) {
      // Retransmit of a package that we have acked before. (Maybe the ack was
      // lost.) Ignoring, but re-triggering the ack.
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
        << seq_id << ", " << in_ring_.start_pos() << "--" << in_ring_.end_pos();
  } else {
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
    } while (in_ring_.contains(unack_seq_) && !getInBuffer(unack_seq_).empty());
  }
}

}  // namespace roo_io