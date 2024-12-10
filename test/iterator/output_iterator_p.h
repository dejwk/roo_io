#pragma once

#include <string>

#include "gtest/gtest.h"

namespace roo_io {

template <typename ItrFactory>
class OutputIteratorTest : public testing::Test {
 public:
  auto createIterator(size_t max_size) {
    return factory.createIterator(max_size);
  }

  std::string getResult() const { return factory.getResult(); }

 private:
  ItrFactory factory;
};

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
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  itr.write(byte{'B'});
  itr.flush();
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  EXPECT_EQ("", this->getResult());
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
  EXPECT_EQ("ABC", this->getResult());
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
    EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
    itr.write(byte{'E'});
    itr.flush();
    EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  }
  EXPECT_EQ("ABC", this->getResult());
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
    EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
    itr.write(byte{'E'});
    itr.flush();
    EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  }
  EXPECT_EQ("ABCDE", this->getResult());
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
    EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
    EXPECT_GE(3, itr.write((const byte*)"GHI", 3));
    itr.flush();
    EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  }
  EXPECT_EQ("ABCDE", this->getResult());
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
  EXPECT_EQ("ABC", this->getResult());
}

REGISTER_TYPED_TEST_SUITE_P(OutputIteratorTest, Initialization, Empty,
                            WriteByByte, WriteByBytePastCapacity,
                            WriteByBlockTillCapacity, WriteByBlockPastCapacity,
                            Movable);

}  // namespace roo_io