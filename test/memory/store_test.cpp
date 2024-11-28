#include "roo_io/memory/store.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "roo_io/memory/load.h"  // For StoreHostNative.

using namespace testing;

namespace roo_io {

TEST(Store, BeU16) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  StoreBeU16(0x4849, data);
  EXPECT_THAT(data, ElementsAre(0x48, 0x49, 9, 9, 9, 9, 9, 9));
}

TEST(Store, LeU16) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  StoreLeU16(0x4849, data);
  EXPECT_THAT(data, ElementsAre(0x49, 0x48, 9, 9, 9, 9, 9, 9));
}

TEST(Store, BeU24) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  StoreBeU24(0x48494A, data);
  EXPECT_THAT(data, ElementsAre(0x48, 0x49, 0x4A, 9, 9, 9, 9, 9));
}

TEST(Store, LeU24) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  StoreLeU24(0x48494A, data);
  EXPECT_THAT(data, ElementsAre(0x4A, 0x49, 0x48, 9, 9, 9, 9, 9));
}

TEST(Store, BeU32) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  StoreBeU32(0x48494A4B, data);
  EXPECT_THAT(data, ElementsAre(0x48, 0x49, 0x4A, 0x4B, 9, 9, 9, 9));
}

TEST(Store, LeU32) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  StoreLeU32(0x48494A4B, data);
  EXPECT_THAT(data, ElementsAre(0x4B, 0x4A, 0x49, 0x48, 9, 9, 9, 9));
}

TEST(Store, BeU64) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  StoreBeU64(0x48494A4B4C4D4E4F, data);
  EXPECT_THAT(
      data, ElementsAre(0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 9, 9));
}

TEST(Store, LeU64) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  StoreLeU64(0x48494A4B4C4D4E4F, data);
  EXPECT_THAT(
      data, ElementsAre(0x4F, 0x4E, 0x4D, 0x4C, 0x4B, 0x4A, 0x49, 0x48, 9, 9));
}

TEST(Store, BeS16) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  StoreBeS16(-2, data);
  EXPECT_THAT(data, ElementsAre(0xFF, 0xFE, 9, 9, 9, 9, 9, 9));
}

TEST(Store, LeS16) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  StoreLeS16(-2, data);
  EXPECT_THAT(data, ElementsAre(0xFE, 0xFF, 9, 9, 9, 9, 9, 9));
}

TEST(Store, BeS24) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  StoreBeS24(-2, data);
  EXPECT_THAT(data, ElementsAre(0xFF, 0xFF, 0xFE, 9, 9, 9, 9, 9));
}

TEST(Store, LeS24) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  StoreLeS24(-2, data);
  EXPECT_THAT(data, ElementsAre(0xFE, 0xFF, 0xFF, 9, 9, 9, 9, 9));
}

TEST(Store, BeS32) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  StoreBeS32(-2, data);
  EXPECT_THAT(data, ElementsAre(0xFF, 0xFF, 0xFF, 0xFE, 9, 9, 9, 9));
}

TEST(Store, LeS32) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  StoreLeS32(-2, data);
  EXPECT_THAT(data, ElementsAre(0xFE, 0xFF, 0xFF, 0xFF, 9, 9, 9, 9));
}

TEST(Store, BeS64) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  StoreBeS64(-2, data);
  EXPECT_THAT(
      data, ElementsAre(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 9, 9));
}

TEST(Store, LeS64) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  StoreLeS64(-2, data);
  EXPECT_THAT(
      data, ElementsAre(0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 9, 9));
}

TEST(Store, Float) {
  float v = 3.14159265;
  roo_io::byte data[sizeof(v)];
  StoreHostNative(v, data);
  float r = LoadHostNative<float>(data);
  EXPECT_EQ(r, v);
}

}  // namespace roo_io