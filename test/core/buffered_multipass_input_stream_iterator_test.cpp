#include "roo_io/core/buffered_multipass_input_stream_iterator.h"

#include <memory>

#include "gtest/gtest.h"
#include "input_iterator_p.h"
#include "multipass_input_iterator_p.h"
#include "roo_io/memory/memory_input_stream.h"

namespace roo_io {

class SeekErrorMultipassInputStream : public MultipassInputStream {
 public:
  SeekErrorMultipassInputStream() : position_(0), status_(kOk) {}

  size_t read(byte* result, size_t count) override {
    if (status_ != kOk) return 0;
    size_t remaining = 128 - position_;
    if (count > remaining) count = remaining;
    memset(result, 0, count);
    position_ += count;
    if (count == 0) status_ = kEndOfStream;
    return count;
  }

  uint64_t size() override { return 128; }

  uint64_t position() const override { return position_; }

  void seek(uint64_t position) override {
    position_ = position;
    status_ = kSeekError;
  }

  void close() override { status_ = kClosed; }

  Status status() const override { return status_; }

 private:
  uint64_t position_;
  Status status_;
};

class BufferedMultipassInputStreamIteratorFixture {
 public:
  using Iterator = BufferedMultipassInputStreamIterator;

  BufferedMultipassInputStreamIterator createIterator(const byte* beg,
                                                      size_t size) {
    is_ = std::unique_ptr<MultipassInputStream>(
        new MemoryInputStream<const byte*>(beg, beg + size));
    return BufferedMultipassInputStreamIterator(*is_);
  }

 private:
  std::unique_ptr<MultipassInputStream> is_;
};

INSTANTIATE_TYPED_TEST_SUITE_P(BufferedMultipassInputStreamIterator,
                               InputIteratorTest,
                               BufferedMultipassInputStreamIteratorFixture);

INSTANTIATE_TYPED_TEST_SUITE_P(BufferedMultipassInputStreamIterator,
                               MultipassInputIteratorTest,
                               BufferedMultipassInputStreamIteratorFixture);

TEST(BufferedMultipassInputStreamIterator, DefaultConstructibleAndClosed) {
  BufferedMultipassInputStreamIterator itr;
  EXPECT_EQ(kClosed, itr.status());
  itr.read();
  EXPECT_EQ(kClosed, itr.status());
  itr.skip(100);
  EXPECT_EQ(kClosed, itr.status());
  byte buf[10];
  EXPECT_EQ(0, itr.read(buf, 10));
  EXPECT_EQ(kClosed, itr.status());
}

TEST(BufferedMultipassInputStreamIterator, DefaultConstructibleResets) {
  BufferedMultipassInputStreamIterator itr;
  EXPECT_EQ(kClosed, itr.status());
  const byte* data = (const byte*)"ABCDEFGH";
  MemoryInputStream<const byte*> is(data, data + 8);
  itr.reset(is);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'A'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
  itr.reset();
  EXPECT_EQ(kClosed, itr.status());
  itr.read();
  EXPECT_EQ(kClosed, itr.status());
}

TEST(BufferedMultipassInputStreamIterator, SeekPreservesUnderlyingFailure) {
  SeekErrorMultipassInputStream input;
  BufferedMultipassInputStreamIterator itr(input);

  itr.seek(1);

  EXPECT_EQ(kSeekError, itr.status());
}

}  // namespace roo_io