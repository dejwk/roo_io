#include "gtest/gtest.h"

namespace roo_io {

template <typename ItrFactory>
class MultipassInputIteratorTest : public testing::Test {
 public:
  auto createIterator(const byte* beg, size_t size) {
    return factory.createIterator(beg, size);
  }

 private:
  ItrFactory factory;
};

TYPED_TEST_SUITE_P(MultipassInputIteratorTest);

TYPED_TEST_P(MultipassInputIteratorTest, Initialization) {
  const byte data[] = "ABCDEFGH";
  auto itr = this->createIterator(data, 8);
  //   EXPECT_EQ(data, itr.ptr());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(8, itr.size());
}

TYPED_TEST_P(MultipassInputIteratorTest, Empty) {
  const byte data[] = "ABCDEFGH";
  auto itr = this->createIterator(data, 0);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(0, itr.size());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(0, itr.size());
}

TYPED_TEST_P(MultipassInputIteratorTest, ReadByByteWithRewind) {
  const byte data[] = "ABCDEFGH";
  auto itr = this->createIterator(data, 3);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('B', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.rewind();
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('B', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('C', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.rewind();
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
}

TYPED_TEST_P(MultipassInputIteratorTest, ReadByByteWithSeek) {
  const byte data[] = "ABCDEFGH";
  auto itr = this->createIterator(data, 3);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.seek(2);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('C', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.seek(1);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('B', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.seek(3);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.seek(1);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('B', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.seek(4);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(4, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.seek(100);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(100, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
  itr.seek(100);
  EXPECT_EQ(kOk, itr.status());
  itr.seek(0);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
}

TYPED_TEST_P(MultipassInputIteratorTest, Movable) {
  const byte data[] = "ABCDEFGH";
  auto itr = this->createIterator(data, 8);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  auto itr2 = std::move(itr);
  EXPECT_EQ('B', itr2.read());
  EXPECT_EQ(kOk, itr2.status());
}

TYPED_TEST_P(MultipassInputIteratorTest, StressTest) {
  size_t size = 1024 * 1024 + 17;
  std::unique_ptr<byte[]> contents(new byte[size]);
  for (size_t i = 0; i < size; ++i) contents[i] = rand() % 256;
  auto itr = this->createIterator(contents.get(), size);
  byte buf[20000];
  ASSERT_EQ(kOk, itr.status());
  size_t pos = 0;
  for (size_t i = 0; i < 10000; i++) {
    ASSERT_EQ(pos, itr.position());
    byte b = itr.read();
    if (pos >= size) {
      ASSERT_EQ(kEndOfStream, itr.status());
    } else {
      ASSERT_EQ(kOk, itr.status());
      ASSERT_EQ(b, contents[pos]);
      ++pos;
    }
    if (rand() % 100 < 10) {
      size_t cnt = rand() % 3000;
      size_t read = itr.read(buf, cnt);
      ASSERT_TRUE(read <= cnt);
      for (size_t i = 0; i < read; ++i) {
        ASSERT_EQ(buf[i], contents[pos + i]);
      }
      pos += read;
    }
    if (rand() % 100 < 2) {
      uint64_t offset = rand() % 2000;
      itr.skip(offset);
      if (offset == 0) {
      } else if (pos + offset > size) {
        ASSERT_EQ(kEndOfStream, itr.status());
        pos = size;
      } else {
        ASSERT_EQ(kOk, itr.status()) << pos;
        pos += offset;
      }
    }
    if (rand() % 1000 < 4) {
      itr.rewind();
      pos = 0;
      ASSERT_EQ(kOk, itr.status());
    } else if (rand() % 1000 < 20) {
      int64_t delta = (rand() % 400) - 200;
      if (delta < -(int64_t)pos) {
        delta = -(int64_t)pos;
      }
      uint64_t newpos = pos + delta;
      itr.seek(newpos);
      pos = newpos;
      ASSERT_EQ(kOk, itr.status());
    }
  }
}

REGISTER_TYPED_TEST_SUITE_P(MultipassInputIteratorTest, Initialization, Empty,
                            ReadByByteWithRewind, ReadByByteWithSeek, Movable,
                            StressTest);

}  // namespace roo_io