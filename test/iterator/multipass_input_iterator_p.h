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
  EXPECT_EQ(data, itr.ptr());
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

REGISTER_TYPED_TEST_SUITE_P(MultipassInputIteratorTest, Initialization, Empty,
                            ReadByByteWithRewind, ReadByByteWithSeek, Movable);

}  // namespace roo_io