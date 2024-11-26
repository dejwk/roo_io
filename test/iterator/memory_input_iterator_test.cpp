#include "roo_io/iterator/memory_input_iterator.h"

#include "gtest/gtest.h"

namespace roo_io {

const uint8_t data[] = "ABCDEFGH";

TEST(UnsafeMemoryIterator, Initialization) {
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(itr.ptr(), data);
}

TEST(UnsafeMemoryIterator, ReadByByte) {
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('B', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('C', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('D', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('E', itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TEST(UnsafeMemoryIterator, ReadArray) {
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ('A', itr.read());
  uint8_t buf[] = "BCD";
  EXPECT_EQ(3, itr.read(buf, 3));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('B', buf[0]);
  EXPECT_EQ('C', buf[1]);
  EXPECT_EQ('D', buf[2]);
  EXPECT_EQ('E', itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TEST(UnsafeMemoryIterator, Skip) {
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ('A', itr.read());
  itr.skip(5);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('G', itr.read());
  EXPECT_EQ(kOk, itr.status());
}

}