#include <memory>

#include "roo_io/core/input_stream.h"
#include "roo_io/core/output_stream.h"
#include "roo_io/memory/load.h"
#include "roo_io/memory/store.h"
#include "roo_io/reliable/packet_receiver.h"
#include "roo_io/reliable/packet_sender.h"
#include "roo_logging.h"

namespace roo_io {

// Helper class to implement reliable bidirectional streaming over lossy
// packet-based transport. Used as a building block of ReliableSerial.
class StreamingRetransmitter {
 public:
  // Can be supplied to be notified when the peer reconnects.
  using ConnectionCb = std::function<void()>;

  class SeqNum {
   public:
    SeqNum(uint16_t seq) : seq_(seq) {}

    bool operator==(SeqNum other) const { return seq_ == other.seq_; }

    bool operator!=(SeqNum other) const { return seq_ != other.seq_; }

    bool operator<(SeqNum other) const {
      return (int16_t)(seq_ - other.seq_) < 0;
    }

    bool operator<=(SeqNum other) const {
      return (int16_t)(seq_ - other.seq_) <= 0;
    }

    bool operator>(SeqNum other) const {
      return (int16_t)(seq_ - other.seq_) > 0;
    }

    bool operator>=(SeqNum other) const {
      return (int16_t)(seq_ - other.seq_) >= 0;
    }

    SeqNum& operator++() {
      ++seq_;
      return *this;
    }

    SeqNum operator++(int) { return SeqNum(seq_++); }

    SeqNum& operator+=(int increment) {
      seq_ += increment;
      return *this;
    }

    int operator-(SeqNum other) const { return (int16_t)(seq_ - other.seq_); }

    SeqNum operator+(int other) const { return SeqNum(seq_ + other); }
    SeqNum operator-(int other) const { return SeqNum(seq_ - other); }

    uint16_t raw() const { return seq_; }

   private:
    uint16_t seq_;
  };

  StreamingRetransmitter(roo_io::PacketSender& sender,
                         roo_io::PacketReceiver& receiver,
                         unsigned int sendbuf_log2, unsigned int recvbuf_log2,
                         ConnectionCb connection_cb = nullptr);

  size_t tryWrite(const roo::byte* buf, size_t count);
  size_t tryRead(roo::byte* buf, size_t count);

  // Returns -1 if no data available to read immediately.
  int peek();

  size_t availableForRead();

  void flush();

  bool conn();
  bool recv() { return receiver_.tryReceive(); }
  void ack();
  bool send();

  bool loop();

  // The lower bound of bytes that are guaranteed to be writable without
  // blocking.
  size_t availableForWrite();

  uint32_t packets_sent() const { return packets_sent_; }
  uint32_t packets_delivered() const { return packets_delivered_; }
  uint32_t packets_received() const { return packets_received_; }

  void reset();

  size_t receiver_bytes_received() const { return receiver_.bytes_received(); }
  size_t receiver_bytes_accepted() const { return receiver_.bytes_accepted(); }

 private:
  class RingBuffer {
   public:
    RingBuffer(int capacity_log2, uint16_t initial_seq = 0)
        : capacity_log2_(capacity_log2),
          begin_(initial_seq),
          end_(initial_seq) {
      CHECK_LE(capacity_log2, 30);
    }

    uint16_t slotsUsed() const { return end_ - begin_; }

    uint16_t slotsFree() const { return capacity() - slotsUsed(); }

    SeqNum begin() const { return begin_; }
    SeqNum end() const { return end_; }

    SeqNum push() {
      CHECK(slotsFree() > 0);
      return end_++;
    }

    SeqNum pop() {
      CHECK(slotsUsed() > 0);
      return begin_++;
    }

    bool empty() const { return slotsUsed() == 0; }

    void reset(SeqNum seq) {
      CHECK_EQ(begin_, end_);
      begin_ = seq;
      end_ = seq;
    }

    // Need to handle wrap-around. But since capacity_log2_ <= 30, on overload,
    // the diffs are going to still be negative.
    bool contains(SeqNum seq) const { return begin_ <= seq && seq < end_; }

    uint16_t offset_for(SeqNum seq) const {
      DCHECK(contains(seq));
      return seq.raw() & (capacity() - 1);
    }

    // Restores high bits of seq, extending it to uint16_t, by assuming that
    // truncated_pos must be 'close' to the range. Specifically, we make sure to
    // pick high bits so that the result is within 1 << (pos_bits/2) from
    // begin.
    SeqNum restorePosHighBits(uint16_t truncated_pos, int pos_bits) {
      DCHECK_GE(pos_bits, capacity_log2_ + 2);
      uint16_t left = begin_.raw() - (1 << (pos_bits - 1));
      return left + (((uint16_t)(truncated_pos - left)) % (1 << pos_bits));
    }

    uint16_t capacity() const { return 1 << capacity_log2_; }

   private:
    uint16_t offset_start() const { return begin_.raw() & (capacity() - 1); }
    uint16_t offset_end() const { return end_.raw() & (capacity() - 1); }

    int capacity_log2_;
    SeqNum begin_;
    SeqNum end_;
  };

  class OutBuffer {
   public:
    OutBuffer() : size_(0), acked_(false), finished_(false) {}

    void reset(SeqNum seq_id);

    bool flushed() const { return flushed_; }
    bool finished() const { return finished_; }
    bool acked() const { return acked_; }

    size_t write(const byte* buf, size_t count) {
      if (finished_) return 0;
      size_t capacity = 248 - size_;
      CHECK_GT(capacity, 0);
      if (count >= capacity) {
        count = capacity;
        flushed_ = true;
        finished_ = true;
        expiration_ = roo_time::Uptime::Start();
      }
      memcpy(payload_ + size_ + 2, buf, count);
      size_ += count;
      return count;
    }

    void flush() { flushed_ = true; }

    void finish() {
      finished_ = true;
      expiration_ = roo_time::Uptime::Start();
    }

    void ack() { acked_ = true; }

    const roo::byte* data() const { return payload_; }
    const uint8_t size() const { return size_ + 2; }

    roo_time::Uptime expiration() const { return expiration_; }

    void markSent(roo_time::Uptime now) {
      if (send_counter_ < 255) ++send_counter_;
      expiration_ = now + roo_time::Millis(10);
    }

    // Updates the timeout of the (already sent) packet to be retransmitted
    // immediately.
    void rush() {
      expiration_ = roo_time::Uptime::Start();
      CHECK_GT(send_counter_, 0);
    }

    // How many times the packet has been already sent.
    uint8_t send_counter() const { return send_counter_; }

   private:
    uint8_t size_;
    bool acked_;
    // Indicates that flush has been requested for this buffer, and therefore,
    // the send loop should transmit it even if it has some more space left.
    bool flushed_;
    // Indicates that no more writes are permitted for this buffer, either
    // because it is already full, or because it has already been transmitted.
    bool finished_;
    // Leave two front bytes for the header (incl. seq number).
    roo::byte payload_[250];

    // Set when sent, to indicate when the packet is due for retransmission.
    roo_time::Uptime expiration_;

    uint8_t send_counter_;
  };

  class InBuffer {
   public:
    InBuffer() : size_(0) {}

    void clear() { size_ = 0; }

    void set(const roo::byte* payload, uint8_t size) {
      CHECK_LE(size, 248);
      memcpy(payload_, payload, size);
      size_ = size;
    }

    bool empty() const { return size_ == 0; }

    const roo::byte* data() const { return payload_; }
    uint8_t size() const { return size_; }

   private:
    uint8_t size_;
    roo::byte payload_[248];
  };

  OutBuffer& getOutBuffer(SeqNum seq) {
    return out_buffers_[out_ring_.offset_for(seq)];
  }

  InBuffer& getInBuffer(SeqNum seq) {
    return in_buffers_[in_ring_.offset_for(seq)];
  }

  void packetReceived(const roo::byte* buf, size_t len);

  roo_io::PacketSender& sender_;
  roo_io::PacketReceiver& receiver_;

  std::unique_ptr<OutBuffer[]> out_buffers_;
  OutBuffer* current_out_buffer_;
  RingBuffer out_ring_;

  // Pointer used to cycle through packets to send, so that we generally send
  // packets in order before trying any retransmissions.
  SeqNum next_to_send_;

  // Ceiling beyond which the receiver currently isn't able to process data.
  // Used in flow control. Updated by the receiver by means of
  // kFlowControlPacket.
  SeqNum recv_himark_;

  std::unique_ptr<InBuffer[]> in_buffers_;
  InBuffer* current_in_buffer_;
  uint8_t current_in_buffer_pos_;
  RingBuffer in_ring_;

  // Whether we need to send kDataAckPacket.
  bool needs_ack_;

  // Newest unacked seq ID.
  uint16_t unack_seq_;

  // Indicates that some receive buffers have been read and freed, increasing
  // the receiver's capacity. Shortly after this flag is set, the receiver will
  // send an update 'kFlowControlPacket' to the sender.
  bool updated_recv_himark_;

  // Random-generated; used in connect packets.
  uint32_t my_stream_id_;

  // As received from the peer in their connect packets.
  uint32_t peer_stream_id_;

  // Indicates whether the peer acknowledged receipt of our stream ID and seq,
  // so that we can start sending messages to it.
  bool sender_connected_;

  // Indicates whether we received the peer's stream ID and seq, allowing us to
  // receive messages from it.
  bool receiver_connected_;

  // Indicates whether we're expected to send the handshake ack message.
  bool needs_handshake_ack_;

  // Used in the handshake backoff protocol.
  uint32_t successive_handshake_retries_;

  roo_time::Uptime next_scheduled_handshake_update_;

  ConnectionCb connection_cb_;
  uint32_t packets_sent_;
  uint32_t packets_delivered_;
  uint32_t packets_received_;
};

}  // namespace roo_io

inline roo_logging::Stream& operator<<(
    roo_logging::Stream& os, roo_io::StreamingRetransmitter::SeqNum seq) {
  os << seq.raw();
  return os;
}