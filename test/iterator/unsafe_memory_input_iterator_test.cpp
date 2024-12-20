#include "gtest/gtest.h"
#include "roo_io/memory/memory_input_iterator.h"

namespace roo_io {

TEST(UnsafeMemoryIterator, Initialization) {
  const byte* data = (const byte*)"ABCDEFGH";
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(itr.ptr(), data);
}

TEST(UnsafeMemoryIterator, ReadByByte) {
  const byte* data = (const byte*)"ABCDEFGH";
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'A'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'B'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'C'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'D'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'E'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TEST(UnsafeMemoryIterator, ReadArray) {
  const byte* data = (const byte*)"ABCDEFGH";
  UnsafeMemoryIterator itr(data);
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

TEST(UnsafeMemoryIterator, Skip) {
  const byte* data = (const byte*)"ABCDEFGH";
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(byte{'A'}, itr.read());
  itr.skip(5);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'G'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TEST(UnsafeMemoryIterator, Movable) {
  const byte* data = (const byte*)"ABCDEFGH";
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(byte{'A'}, itr.read());
  EXPECT_EQ(kOk, itr.status());
  UnsafeMemoryIterator itr2 = std::move(itr);
  EXPECT_EQ(byte{'B'}, itr2.read());
  EXPECT_EQ(kOk, itr2.status());
}

}  // namespace roo_io