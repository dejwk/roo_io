#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "roo_io/data/write.h"
#include "roo_io/iterator/memory_output_iterator.h"
#include "roo_io/memory/load.h"  // ForHostNativeWriter.

using namespace testing;

namespace roo_io {

TEST(Write, BeU16) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 8);
  WriteBeU16(itr, 0x4849);
  EXPECT_THAT(data, ElementsAre(0x48, 0x49, 9, 9, 9, 9, 9, 9));
}

TEST(Write, LeU16) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 8);
  WriteLeU16(itr, 0x4849);
  EXPECT_THAT(data, ElementsAre(0x49, 0x48, 9, 9, 9, 9, 9, 9));
}

TEST(Write, BeU24) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 8);
  WriteBeU24(itr, 0x48494A);
  EXPECT_THAT(data, ElementsAre(0x48, 0x49, 0x4A, 9, 9, 9, 9, 9));
}

TEST(Write, LeU24) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 8);
  WriteLeU24(itr, 0x48494A);
  EXPECT_THAT(data, ElementsAre(0x4A, 0x49, 0x48, 9, 9, 9, 9, 9));
}

TEST(Write, BeU32) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 8);
  WriteBeU32(itr, 0x48494A4B);
  EXPECT_THAT(data, ElementsAre(0x48, 0x49, 0x4A, 0x4B, 9, 9, 9, 9));
}

TEST(Write, LeU32) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 8);
  WriteLeU32(itr, 0x48494A4B);
  EXPECT_THAT(data, ElementsAre(0x4B, 0x4A, 0x49, 0x48, 9, 9, 9, 9));
}

TEST(Write, BeU64) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 10);
  WriteBeU64(itr, 0x48494A4B4C4D4E4F);
  EXPECT_THAT(
      data, ElementsAre(0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 9, 9));
}

TEST(Write, LeU64) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 10);
  WriteLeU64(itr, 0x48494A4B4C4D4E4F);
  EXPECT_THAT(
      data, ElementsAre(0x4F, 0x4E, 0x4D, 0x4C, 0x4B, 0x4A, 0x49, 0x48, 9, 9));
}

TEST(Write, BeS16) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 8);
  WriteBeS16(itr, -2);
  EXPECT_THAT(data, ElementsAre(0xFF, 0xFE, 9, 9, 9, 9, 9, 9));
}

TEST(Write, LeS16) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 8);
  WriteLeS16(itr, -2);
  EXPECT_THAT(data, ElementsAre(0xFE, 0xFF, 9, 9, 9, 9, 9, 9));
}

TEST(Write, BeS24) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 8);
  WriteBeS24(itr, -2);
  EXPECT_THAT(data, ElementsAre(0xFF, 0xFF, 0xFE, 9, 9, 9, 9, 9));
}

TEST(Write, LeS24) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 8);
  WriteLeS24(itr, -2);
  EXPECT_THAT(data, ElementsAre(0xFE, 0xFF, 0xFF, 9, 9, 9, 9, 9));
}

TEST(Write, BeS32) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 8);
  WriteBeS32(itr, -2);
  EXPECT_THAT(data, ElementsAre(0xFF, 0xFF, 0xFF, 0xFE, 9, 9, 9, 9));
}

TEST(Write, LeS32) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 8);
  WriteLeS32(itr, -2);
  EXPECT_THAT(data, ElementsAre(0xFE, 0xFF, 0xFF, 0xFF, 9, 9, 9, 9));
}

TEST(Write, BeS64) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 10);
  WriteBeS64(itr, -2);
  EXPECT_THAT(
      data, ElementsAre(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 9, 9));
}

TEST(Write, LeS64) {
  byte data[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr(data, data + 10);
  WriteLeS64(itr, -2);
  EXPECT_THAT(
      data, ElementsAre(0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 9, 9));
}

TEST(Write, Float) {
  float v = 3.14159265;
  byte data[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr(data, data + 8);
  HostNativeWriter<float>().write(itr, v);
  float r = LoadHostNative<float>(data);
  EXPECT_EQ(r, v);
  EXPECT_EQ(0, data[5]);
}

TEST(Write, VarU64_0) {
  byte data[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr(data, data + 8);
  WriteVarU64(itr, 0);
  EXPECT_THAT(data, ElementsAre(0, 0, 0, 0, 0, 0, 0, 0));
}

TEST(Write, VarU64_1) {
  byte data[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr(data, data + 8);
  WriteVarU64(itr, 1);
  EXPECT_THAT(data, ElementsAre(1, 0, 0, 0, 0, 0, 0, 0));
}

TEST(Write, VarU64_127) {
  byte data[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr(data, data + 8);
  WriteVarU64(itr, 127);
  EXPECT_THAT(data, ElementsAre(0x7F, 0, 0, 0, 0, 0, 0, 0));
}

TEST(Write, VarU64_128) {
  byte data[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr(data, data + 8);
  WriteVarU64(itr, 128);
  EXPECT_THAT(data, ElementsAre(0x80, 0x01, 0, 0, 0, 0, 0, 0));
}

TEST(Write, VarU64_150) {
  byte data[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr(data, data + 8);
  WriteVarU64(itr, 150);
  EXPECT_THAT(data, ElementsAre(0x96, 0x01, 0, 0, 0, 0, 0, 0));
}

struct DrippingIterator {
  byte* data;
  byte* end;
  size_t write(const byte* buf, size_t count) {
    if (count > 3) count = 3;
    if (count > end - data) count = end - data;
    memcpy(data, buf, count);
    data += count;
    return count;
  }
};

TEST(Write, ByteArray) {
  byte result[] = {0, 0, 0, 0, 0, 0, 0, 0};
  DrippingIterator itr{result, result + 8};
  byte in[] = {1, 2, 3, 4, 5, 6, 7, 8};
  EXPECT_EQ(5, WriteByteArray(itr, in, 5));
  EXPECT_THAT(result, ElementsAre(1, 2, 3, 4, 5, 0, 0, 0));
}

TEST(Write, ByteArrayOverflow) {
  byte result[] = {0, 0, 0, 0, 0, 0, 0, 0};
  DrippingIterator itr{result, result + 4};
  byte in[] = {1, 2, 3, 4, 5, 6, 7, 8};
  EXPECT_EQ(4, WriteByteArray(itr, in, 10));
  EXPECT_THAT(result, ElementsAre(1, 2, 3, 4, 0, 0, 0, 0));
}

TEST(Write, NativeOverflow) {
  byte result[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr{result, result + 2};
  float f = 3.14159265;
  HostNativeWriter<float>().write(itr, f);
  EXPECT_EQ(0, result[3]);
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
}

}  // namespace roo_io