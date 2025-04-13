#include <memory>

#include "roo_io/core/input_stream.h"
#include "roo_io/core/output_stream.h"
#include "roo_io/memory/load.h"
#include "roo_io/memory/store.h"
#include "roo_io/reliable/packet_receiver.h"
#include "roo_io/reliable/packet_sender.h"

namespace roo_io {

// Helper class to implement reliable bidirectional streaming over lossy
// packet-based transport. Used as a building block of ReliableSerial.
class StreamingRetransmitter {
 public:
  StreamingRetransmitter(roo_io::PacketSender& sender,
                         roo_io::PacketReceiver& receiver,
                         unsigned int sendbuf_log2, unsigned int recvbuf_log2);

  size_t tryWrite(const roo::byte* buf, size_t count);

  size_t tryRead(roo::byte* buf, size_t count);

  void flush();

  bool sendLoop();
  void recvLoop() { receiver_.tryReceive(); }

 private:
  class RingBuffer {
   public:
    RingBuffer(int capacity_log2, int initial_pos = 0)
        : capacity_log2_(capacity_log2),
          start_pos_(initial_pos),
          end_pos_(initial_pos) {
      CHECK_LE(capacity_log2, 30);
    }

    uint32_t slotsUsed() const { return end_pos_ - start_pos_; }

    uint32_t slotsFree() const { return capacity() - slotsUsed(); }

    uint32_t end_pos() const { return end_pos_; }
    uint32_t start_pos() const { return start_pos_; }

    uint32_t push() {
      CHECK(slotsFree() > 0);
      return end_pos_++;
    }

    void push(uint32_t count) {
      CHECK(slotsFree() >= count);
      end_pos_ += count;
    }

    uint32_t pop() {
      CHECK(slotsUsed() > 0);
      return start_pos_++;
    }

    bool empty() const { return slotsUsed() == 0; }

    // Need to handle wrap-around. But since capacity_log2_ <= 30, on overload,
    // the diffs are going to still be negative.
    bool contains(uint32_t pos) const {
      return ((int32_t)(pos - start_pos_)) >= 0 &&
             ((int32_t)(end_pos_ - pos)) > 0;
    }

    uint32_t offset_for(uint32_t pos) const {
      CHECK(contains(pos));
      return pos & (capacity() - 1);
    }

    // Restores high bits of pos, extending it to uint32_t, by assuming that
    // truncated_pos must be 'close' to the range. Specifically, we make sure to
    // pick high bits so that the result is within 1 << (pos_bits/2) from
    // start_pos.
    uint32_t restorePosHighBits(uint32_t truncated_pos, int pos_bits) {
      DCHECK_GE(pos_bits, capacity_log2_ + 2);
      uint32_t left = start_pos() - (1 << (pos_bits - 1));
      return left + ((truncated_pos - left) % (1 << pos_bits));
    }

   private:
    uint32_t capacity() const { return 1 << capacity_log2_; }

    uint32_t offset_start() const { return start_pos_ & (capacity() - 1); }
    uint32_t offset_end() const { return end_pos_ & (capacity() - 1); }

    int capacity_log2_;
    uint32_t start_pos_;
    uint32_t end_pos_;
  };

  class OutBuffer {
   public:
    OutBuffer() : size_(0), acked_(false), finished_(false) {}

    void reset(uint32_t seq_id) {
      uint16_t header = seq_id & 0x0FFF;
      roo_io::StoreBeU16(header, payload_);
      size_ = 0;
      acked_ = false;
      finished_ = false;
    }

    bool finished() const { return finished_; }
    bool acked() const { return acked_; }

    size_t write(const byte* buf, size_t count) {
      CHECK(!finished_);
      size_t capacity = 248 - size_;
      CHECK_GT(capacity, 0);
      if (count >= capacity) {
        count = capacity;
        finished_ = true;
      }
      memcpy(payload_ + size_ + 2, buf, count);
      size_ += count;
      return count;
    }

    void finish() { finished_ = true; }

    void ack() { acked_ = true; }

    const roo::byte* data() const { return payload_; }
    const uint8_t size() const { return size_ + 2; }

   private:
    uint8_t size_;
    bool acked_;
    bool finished_;
    // Leave two front bytes for the header (incl. seq number).
    roo::byte payload_[250];
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

  OutBuffer& getOutBuffer(uint32_t id) {
    return out_buffers_[out_ring_.offset_for(id)];
  }

  InBuffer& getInBuffer(uint32_t id) {
    return in_buffers_[in_ring_.offset_for(id)];
  }

  void packetReceived(const roo::byte* buf, size_t len);

  roo_io::PacketSender& sender_;
  roo_io::PacketReceiver& receiver_;

  int sendbuf_capacity_;
  std::unique_ptr<OutBuffer[]> out_buffers_;
  OutBuffer* current_out_buffer_;
  RingBuffer out_ring_;
  uint32_t next_to_send_;
  int available_tokens_;

  int recvbuf_capacity_;
  std::unique_ptr<InBuffer[]> in_buffers_;
  InBuffer* current_in_buffer_;
  uint8_t current_in_buffer_pos_;
  RingBuffer in_ring_;

  bool needs_ack_;
  // Newest unacked seq ID.
  uint32_t unack_seq_;

  bool needs_token_send_;
};

}  // namespace roo_io