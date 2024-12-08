#include "gtest/gtest.h"
#include "roo_io/iterator/memory_input_iterator.h"

namespace roo_io {

TEST(UnsafeMemoryIterator, Initialization) {
  const byte data[] = "ABCDEFGH";
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(itr.ptr(), data);
}

TEST(UnsafeMemoryIterator, ReadByByte) {
  const byte data[] = "ABCDEFGH";
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
  const byte data[] = "ABCDEFGH";
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ('A', itr.read());
  byte buf[] = "BCD";
  EXPECT_EQ(3, itr.read(buf, 3));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('B', buf[0]);
  EXPECT_EQ('C', buf[1]);
  EXPECT_EQ('D', buf[2]);
  EXPECT_EQ('E', itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TEST(UnsafeMemoryIterator, Skip) {
  const byte data[] = "ABCDEFGH";
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ('A', itr.read());
  itr.skip(5);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('G', itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TEST(UnsafeMemoryIterator, Movable) {
  const byte data[] = "ABCDEFGH";
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  UnsafeMemoryIterator itr2 = std::move(itr);
  EXPECT_EQ('B', itr2.read());
  EXPECT_EQ(kOk, itr2.status());
}

}  // namespace roo_io