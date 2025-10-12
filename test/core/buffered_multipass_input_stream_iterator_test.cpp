#include "roo_io/core/buffered_multipass_input_stream_iterator.h"

#include <memory>

#include "gtest/gtest.h"
#include "input_iterator_p.h"
#include "multipass_input_iterator_p.h"
#include "roo_io/memory/memory_input_stream.h"

namespace roo_io {

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

}  // namespace roo_io