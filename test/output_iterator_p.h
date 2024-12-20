#pragma once

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace roo_io {

template <typename ItrFactory>
class OutputIteratorTest : public testing::Test {
 public:
  auto createIterator(size_t max_size) {
    return factory.createIterator(max_size);
  }

  std::vector<byte> getResult() const { return factory.getResult(); }

  std::string getResultAsString() const { return factory.getResultAsString(); }

 private:
  ItrFactory factory;
};

// void PrintTo(byte b, std::ostream* os) {
//   *os << (uint8_t)b;
// }

MATCHER_P(IsNoSpaceLeftOnDevice, strict, "") {
  return strict ? arg == kNoSpaceLeftOnDevice
                : arg == kNoSpaceLeftOnDevice || arg == kWriteError;
}

TYPED_TEST_SUITE_P(OutputIteratorTest);

TYPED_TEST_P(OutputIteratorTest, Initialization) {
  auto itr = this->createIterator(5);
  EXPECT_EQ(kOk, itr.status());
}

TYPED_TEST_P(OutputIteratorTest, Empty) {
  auto itr = this->createIterator(0);
  //   EXPECT_EQ(buf, itr.ptr());
  EXPECT_EQ(kOk, itr.status());
  itr.write(byte{'A'});
  itr.flush();
  EXPECT_THAT(itr.status(), IsNoSpaceLeftOnDevice(TypeParam::strict));
  itr.write(byte{'B'});
  itr.flush();
  EXPECT_THAT(itr.status(), IsNoSpaceLeftOnDevice(TypeParam::strict));
  EXPECT_EQ("", this->getResultAsString());
}

TYPED_TEST_P(OutputIteratorTest, WriteByByte) {
  {
    auto itr = this->createIterator(8);
    //   EXPECT_EQ(buf, itr.ptr());
    EXPECT_EQ(kOk, itr.status());
    itr.write(byte{'A'});
    EXPECT_EQ(kOk, itr.status());
    itr.write(byte{'B'});
    EXPECT_EQ(kOk, itr.status());
    itr.write(byte{'C'});
    EXPECT_EQ(kOk, itr.status());
  }
  EXPECT_EQ("ABC", this->getResultAsString());
}

TYPED_TEST_P(OutputIteratorTest, WriteByBytePastCapacity) {
  {
    auto itr = this->createIterator(3);
    //   EXPECT_EQ(buf, itr.ptr());
    EXPECT_EQ(kOk, itr.status());
    itr.write(byte{'A'});
    EXPECT_EQ(kOk, itr.status());
    itr.write(byte{'B'});
    EXPECT_EQ(kOk, itr.status());
    itr.write(byte{'C'});
    EXPECT_EQ(kOk, itr.status());
    itr.write(byte{'D'});
    itr.flush();
    EXPECT_THAT(itr.status(), IsNoSpaceLeftOnDevice(TypeParam::strict));
    itr.write(byte{'E'});
    itr.flush();
    EXPECT_THAT(itr.status(), IsNoSpaceLeftOnDevice(TypeParam::strict));
  }
  EXPECT_EQ("ABC", this->getResultAsString());
}

TYPED_TEST_P(OutputIteratorTest, WriteByBlockTillCapacity) {
  {
    auto itr = this->createIterator(5);
    //   EXPECT_EQ(buf, itr.ptr());
    EXPECT_EQ(kOk, itr.status());
    EXPECT_EQ(3, itr.write((const byte*)"ABC", 3));
    EXPECT_EQ(kOk, itr.status());
    EXPECT_EQ(2, itr.write((const byte*)"DE", 2));
    EXPECT_EQ(kOk, itr.status());
    itr.write(byte{'D'});
    itr.flush();
    EXPECT_THAT(itr.status(), IsNoSpaceLeftOnDevice(TypeParam::strict));
    itr.write(byte{'E'});
    itr.flush();
    EXPECT_THAT(itr.status(), IsNoSpaceLeftOnDevice(TypeParam::strict));
  }
  EXPECT_EQ("ABCDE", this->getResultAsString());
}

TYPED_TEST_P(OutputIteratorTest, WriteByBlockPastCapacity) {
  {
    auto itr = this->createIterator(5);
    //   EXPECT_EQ(buf, itr.ptr());
    EXPECT_EQ(kOk, itr.status());
    EXPECT_EQ(3, itr.write((const byte*)"ABC", 3));
    EXPECT_EQ(kOk, itr.status());
    EXPECT_GE(3, itr.write((const byte*)"DEF", 3));
    itr.flush();
    EXPECT_THAT(itr.status(), IsNoSpaceLeftOnDevice(TypeParam::strict));
    EXPECT_GE(3, itr.write((const byte*)"GHI", 3));
    itr.flush();
    EXPECT_THAT(itr.status(), IsNoSpaceLeftOnDevice(TypeParam::strict));
  }
  EXPECT_EQ("ABCDE", this->getResultAsString());
}

TYPED_TEST_P(OutputIteratorTest, Movable) {
  {
    auto itr = this->createIterator(8);
    //   EXPECT_EQ(buf, itr.ptr());
    EXPECT_EQ(kOk, itr.status());
    itr.write(byte{'A'});
    EXPECT_EQ(kOk, itr.status());
    auto itr2 = std::move(itr);
    EXPECT_EQ(kOk, itr2.status());
    itr2.write(byte{'B'});
    EXPECT_EQ(kOk, itr2.status());
    itr2.write(byte{'C'});
    EXPECT_EQ(kOk, itr2.status());
  }
  EXPECT_EQ("ABC", this->getResultAsString());
}

TYPED_TEST_P(OutputIteratorTest, StressTest) {
  size_t size = 1024 * 1024 + 17;
  std::unique_ptr<byte[]> contents(new byte[size]);
  uint64_t pos = 0;
  {
    auto itr = this->createIterator(size);
    ASSERT_EQ(kOk, itr.status());
    byte buf[20000];
    for (size_t i = 0; i < 10000; i++) {
      if (pos >= size) break;
      byte b = (byte)rand();
      contents[pos++] = b;
      itr.write(b);
      EXPECT_EQ(kOk, itr.status());
      if (rand() % 100 < 10) {
        size_t cnt = rand() % 3000;
        if (cnt > size - pos) cnt = size - pos;
        for (size_t j = 0; j < cnt; ++j) {
          buf[j] = (byte)rand();
        }
        memcpy(&contents[pos], buf, cnt);
        pos += cnt;
        byte* pos = buf;
        while (cnt > 0) {
          size_t written = itr.write(pos, cnt);
          ASSERT_TRUE(written <= cnt);
          pos += written;
          cnt -= written;
        }
      }
    }
  }
  EXPECT_EQ(std::vector<byte>(&contents[0], &contents[pos]), this->getResult());
}

REGISTER_TYPED_TEST_SUITE_P(OutputIteratorTest, Initialization, Empty,
                            WriteByByte, WriteByBytePastCapacity,
                            WriteByBlockTillCapacity, WriteByBlockPastCapacity,
                            Movable, StressTest);

}  // namespace roo_io