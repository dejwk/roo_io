#include "roo_io/memory/store.h"

// For StoreHostNative.
#include "roo_io/memory/load.h"

#include "gtest/gtest.h"

uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};

namespace roo_io {

TEST(Store, BeU16) {
  char data[] = "EAEAEAEA";
  StoreBeU16(0x4849, (uint8_t*)data);
  EXPECT_STREQ("HIEAEAEA", data);
}

TEST(Store, LeU16) {
  char data[] = "EAEAEAEA";
  StoreLeU16(0x4849, (uint8_t*)data);
  EXPECT_STREQ("IHEAEAEA", data);
}

TEST(Store, BeU24) {
  char data[] = "EAEAEAEA";
  StoreBeU24(0x48494A, (uint8_t*)data);
  EXPECT_STREQ("HIJAEAEA", data);
}

TEST(Store, LeU24) {
  char data[] = "EAEAEAEA";
  StoreLeU24(0x48494A, (uint8_t*)data);
  EXPECT_STREQ("JIHAEAEA", data);
}

TEST(Store, BeU32) {
  char data[] = "EAEAEAEA";
  StoreBeU32(0x48494A4B, (uint8_t*)data);
  EXPECT_STREQ("HIJKEAEA", data);
}

TEST(Store, LeU32) {
  char data[] = "EAEAEAEA";
  StoreLeU32(0x48494A4B, (uint8_t*)data);
  EXPECT_STREQ("KJIHEAEA", data);
}

TEST(Store, BeU64) {
  char data[] = "EAEAEAEAEA";
  StoreBeU64(0x48494A4B4C4D4E4F, (uint8_t*)data);
  EXPECT_STREQ("HIJKLMNOEA", data);
}

TEST(Store, LeU64) {
  char data[] = "EAEAEAEAEA";
  StoreLeU64(0x48494A4B4C4D4E4F, (uint8_t*)data);
  EXPECT_STREQ("ONMLKJIHEA", data);
}

TEST(Store, BeS16) {
  char data[] = "EAEAEAEA";
  StoreBeS16(-2, (uint8_t*)data);
  EXPECT_STREQ("\xFF\xFE" "EAEAEA", data);
}

TEST(Store, LeS16) {
  char data[] = "EAEAEAEA";
  StoreLeS16(-2, (uint8_t*)data);
  EXPECT_STREQ("\xFE\xFF" "EAEAEA", data);
}

TEST(Store, BeS24) {
  char data[] = "EAEAEAEA";
  StoreBeS24(-2, (uint8_t*)data);
  EXPECT_STREQ("\xFF\xFF\xFE" "AEAEA", data);
}

TEST(Store, LeS24) {
  char data[] = "EAEAEAEA";
  StoreLeS24(-2, (uint8_t*)data);
  EXPECT_STREQ("\xFE\xFF\xFF" "AEAEA", data);
}

TEST(Store, BeS32) {
  char data[] = "EAEAEAEA";
  StoreBeS32(-2, (uint8_t*)data);
  EXPECT_STREQ("\xFF\xFF\xFF\xFE" "EAEA", data);
}

TEST(Store, LeS32) {
  char data[] = "EAEAEAEA";
  StoreLeS32(-2, (uint8_t*)data);
  EXPECT_STREQ("\xFE\xFF\xFF\xFF" "EAEA", data);
}

TEST(Store, BeS64) {
  char data[] = "EAEAEAEAEA";
  StoreBeS64(-2, (uint8_t*)data);
  EXPECT_STREQ("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFE" "EA", data);
}

TEST(Store, LeS64) {
  char data[] = "EAEAEAEAEA";
  StoreLeS64(-2, (uint8_t*)data);
  EXPECT_STREQ("\xFE\xFF\xFF\xFF\xFF\xFF\xFF\xFF" "EA", data);
}

TEST(Store, Float) {
  float v = 3.14159265;
  uint8_t data[sizeof(v)];
  StoreHostNative(v, data);
  float r = LoadHostNative<float>(data);
  EXPECT_EQ(r, v);
}

}  // namespace roo_io