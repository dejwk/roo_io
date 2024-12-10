#pragma once

#include "gtest/gtest.h"
#include "roo_io/base/byte.h"
#include "roo_io/status.h"

namespace roo_io {

template <typename ItrFactory>
class InputIteratorTest : public testing::Test {
 public:
  typename ItrFactory::Iterator createIterator(const byte* beg, size_t size) {
    return factory.createIterator(beg, size);
  }

  typename ItrFactory::Iterator createIteratorAdvanceAndReturn(const byte* beg,
                                                               size_t size) {
    auto itr = createIterator(beg, size);
    EXPECT_EQ(kOk, itr.status());
    EXPECT_EQ(*beg, itr.read());
    EXPECT_EQ(kOk, itr.status());
    auto itr2 = std::move(itr);
    return itr2;
  }

 private:
  ItrFactory factory;
};

TYPED_TEST_SUITE_P(InputIteratorTest);

TYPED_TEST_P(InputIteratorTest, Empty) {
  const byte* data = (const byte*)"ABCDEFGH";
  auto itr = this->createIterator(data, 0);
  EXPECT_EQ(kOk, itr.status());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
}

TYPED_TEST_P(InputIteratorTest, ReadByByte) {
  const byte* data = (const byte*)"ABCDEFGH";
  auto itr = this->createIterator(data, 3);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'A'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'B'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'C'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
}

TYPED_TEST_P(InputIteratorTest, ReadArray) {
  const byte* data = (const byte*)"ABCDEFGH";
  auto itr = this->createIterator(data, 8);
  EXPECT_EQ(byte{'A'}, itr.read());
  byte buf[3];
  EXPECT_EQ(3, itr.read(buf, 3));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'B'}, buf[0]);
  EXPECT_EQ(byte{'C'}, buf[1]);
  EXPECT_EQ(byte{'D'}, buf[2]);
  EXPECT_EQ(byte{'E'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TYPED_TEST_P(InputIteratorTest, ReadArrayPastEos) {
  const byte* data = (const byte*)"ABCDEFGH";
  auto itr = this->createIterator(data, 3);
  EXPECT_EQ(byte{'A'}, itr.read());
  byte buf[3];
  EXPECT_EQ(2, itr.read(buf, 3));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'B'}, buf[0]);
  EXPECT_EQ(byte{'C'}, buf[1]);
  EXPECT_EQ(0, itr.read(buf, 3));
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(0, itr.read(buf, 3));
  EXPECT_EQ(kEndOfStream, itr.status());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
}

TYPED_TEST_P(InputIteratorTest, Skip) {
  const byte* data = (const byte*)"ABCDEFGH";
  auto itr = this->createIterator(data, 8);
  EXPECT_EQ(byte{'A'}, itr.read());
  itr.skip(5);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'G'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TYPED_TEST_P(InputIteratorTest, SkipPastEos) {
  const byte* data = (const byte*)"ABCDEFGH";
  auto itr = this->createIterator(data, 8);
  EXPECT_EQ(byte{'A'}, itr.read());
  itr.skip(5);
  EXPECT_EQ(kOk, itr.status());
  itr.skip(5);
  EXPECT_EQ(kEndOfStream, itr.status());
  itr.skip(3);
  EXPECT_EQ(kEndOfStream, itr.status());
}

TYPED_TEST_P(InputIteratorTest, Movable) {
  const byte* data = (const byte*)"ABCDEFGH";
  auto itr = this->createIteratorAdvanceAndReturn(data, 8);
  // EXPECT_EQ(kOk, itr.status());
  // EXPECT_EQ(byte{'A'}, itr.read());
  // EXPECT_EQ(kOk, itr.status());
  // auto itr2 = std::move(itr);
  EXPECT_EQ(byte{'B'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TYPED_TEST_P(InputIteratorTest, StressTest) {
  size_t size = 1024 * 1024 + 17;
  std::unique_ptr<byte[]> contents(new byte[size]);
  for (size_t i = 0; i < size; ++i) contents[i] = (byte)rand();
  auto itr = this->createIterator(contents.get(), size);
  byte buf[20000];
  ASSERT_EQ(kOk, itr.status());
  size_t pos = 0;
  for (size_t i = 0; i < 10000; i++) {
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
        ASSERT_EQ(kOk, itr.status());
        pos += offset;
      }
    }
  }
}

REGISTER_TYPED_TEST_SUITE_P(InputIteratorTest, Empty, ReadByByte, ReadArray,
                            ReadArrayPastEos, Skip, SkipPastEos, Movable,
                            StressTest);

}  // namespace roo_io
