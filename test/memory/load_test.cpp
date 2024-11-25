#include "roo_io/memory/load.h"

#include "gtest/gtest.h"

#include "roo_io/memory/load.h"

uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};

namespace roo_io {

TEST(Load, Unsigned) {
  EXPECT_EQ(LoadBeU16(data), 0x1234);
  EXPECT_EQ(LoadLeU16(data), 0x3412);
  EXPECT_EQ(LoadBeU24(data), 0x123456);
  EXPECT_EQ(LoadLeU24(data), 0x563412);
  EXPECT_EQ(LoadBeU32(data), 0x12345678);
  EXPECT_EQ(LoadLeU32(data), 0x78563412);
  EXPECT_EQ(LoadBeU64(data), 0x123456789ABCDEF0LL);
  EXPECT_EQ(LoadLeU64(data), 0xF0DEBC9A78563412LL);
}

TEST(Load, SignedSimple) {
  EXPECT_EQ(LoadBeS16(data), 0x1234);
  EXPECT_EQ(LoadLeS16(data), 0x3412);
  EXPECT_EQ(LoadBeS24(data), 0x123456);
  EXPECT_EQ(LoadLeS24(data), 0x563412);
  EXPECT_EQ(LoadBeS32(data), 0x12345678);
  EXPECT_EQ(LoadLeS32(data), 0x78563412);
  EXPECT_EQ(LoadBeS64(data), 0x123456789ABCDEF0LL);
  EXPECT_EQ(LoadLeS64(data), 0xF0DEBC9A78563412LL);
}

TEST(Load, SignedNegative) {
  uint8_t neg1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  EXPECT_EQ(LoadBeS16(neg1), -1);
  EXPECT_EQ(LoadLeS16(neg1), -1);
  EXPECT_EQ(LoadBeS24(neg1), -1);
  EXPECT_EQ(LoadLeS24(neg1), -1);
  EXPECT_EQ(LoadBeS32(neg1), -1);
  EXPECT_EQ(LoadLeS32(neg1), -1);
  EXPECT_EQ(LoadBeS64(neg1), -1);
  EXPECT_EQ(LoadLeS64(neg1), -1);
}

TEST(Load, Float) {
  float num = 34664315.451;
  EXPECT_EQ(num, LoadFloat((const uint8_t*)(const char*)&num));
}

TEST(Load, Double) {
  double num = 34664315.451;
  EXPECT_EQ(num, LoadDouble((const uint8_t*)(const char*)&num));
}

}  // namespace roo_io