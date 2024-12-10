#include "roo_io/iterable/memory_iterable.h"

#include "gtest/gtest.h"

namespace roo_io {

const byte* data = (const byte*)"ABCDEFGH";

TEST(UnsafeMemoryIterable, Basic) {
  UnsafeMemoryIterable obj(data);
  // Test iterator independence.
  auto itr1 = obj.iterator();
  auto itr2 = obj.iterator();
  EXPECT_EQ(byte{'A'}, itr1.read());
  EXPECT_EQ(byte{'B'}, itr1.read());
  EXPECT_EQ(kOk, itr1.status());
  EXPECT_EQ(byte{'A'}, itr2.read());
  EXPECT_EQ(byte{'B'}, itr2.read());
  EXPECT_EQ(kOk, itr2.status());
}

TEST(SafeMemoryIterable, BasicWithOverload) {
  MemoryIterable obj(data, data + 2);
  auto itr1 = obj.iterator();
  auto itr2 = obj.iterator();
  EXPECT_EQ(byte{'A'}, itr1.read());
  EXPECT_EQ(byte{'B'}, itr1.read());
  itr1.read();
  EXPECT_EQ(kEndOfStream, itr1.status());
  EXPECT_EQ(kOk, itr2.status());
  EXPECT_EQ(byte{'A'}, itr2.read());
  EXPECT_EQ(byte{'B'}, itr2.read());
  EXPECT_EQ(kOk, itr2.status());
}

TEST(MultipassMemoryIterable, Basic) {
  MultipassMemoryIterable obj(data, data + 3);
  auto itr1 = obj.iterator();
  auto itr2 = obj.iterator();
  EXPECT_EQ(kOk, itr1.status());
  EXPECT_EQ(0, itr1.position());
  EXPECT_EQ(3, itr1.size());
  EXPECT_EQ(byte{'A'}, itr1.read());
  EXPECT_EQ(kOk, itr1.status());
  EXPECT_EQ(1, itr1.position());
  EXPECT_EQ(3, itr1.size());
  EXPECT_EQ(byte{'B'}, itr1.read());
  EXPECT_EQ(kOk, itr1.status());
  EXPECT_EQ(2, itr1.position());
  EXPECT_EQ(3, itr1.size());
  EXPECT_EQ(kOk, itr2.status());
  EXPECT_EQ(0, itr2.position());
  EXPECT_EQ(3, itr2.size());
  EXPECT_EQ(byte{'A'}, itr2.read());
  EXPECT_EQ(kOk, itr2.status());
  EXPECT_EQ(1, itr2.position());
  EXPECT_EQ(3, itr2.size());
  EXPECT_EQ(byte{'B'}, itr2.read());
  EXPECT_EQ(kOk, itr2.status());
  EXPECT_EQ(2, itr2.position());
  EXPECT_EQ(3, itr2.size());
}

}
