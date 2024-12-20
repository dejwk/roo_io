#include "roo_io/data/read.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "roo_io/memory/memory_input_iterator.h"

using testing::ElementsAre;

namespace roo_io {

TEST(Read, Unsigned) {
  byte data[] = {byte{0x12}, byte{0x34}, byte{0x56}, byte{0x78},
                 byte{0x9A}, byte{0xBC}, byte{0xDE}, byte{0xF0}};
  MultipassMemoryIterator itr(data, data + 8);
  EXPECT_EQ(ReadBeU16(itr), 0x1234);
  itr.rewind();
  EXPECT_EQ(ReadLeU16(itr), 0x3412);
  itr.rewind();
  EXPECT_EQ(ReadBeU24(itr), 0x123456);
  itr.rewind();
  EXPECT_EQ(ReadLeU24(itr), 0x563412);
  itr.rewind();
  EXPECT_EQ(ReadBeU32(itr), 0x12345678);
  itr.rewind();
  EXPECT_EQ(ReadLeU32(itr), 0x78563412);
  itr.rewind();
  EXPECT_EQ(ReadBeU64(itr), 0x123456789ABCDEF0LL);
  itr.rewind();
  EXPECT_EQ(ReadLeU64(itr), 0xF0DEBC9A78563412LL);
  itr.rewind();
}

TEST(Read, UnsignedTemplated) {
  byte data[] = {byte{0x12}, byte{0x34}, byte{0x56}, byte{0x78},
                 byte{0x9A}, byte{0xBC}, byte{0xDE}, byte{0xF0}};
  MultipassMemoryIterator itr(data, data + 8);
  EXPECT_EQ(IntegerReader<kBigEndian>().readU16(itr), 0x1234);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kLittleEndian>().readU16(itr), 0x3412);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kBigEndian>().readU24(itr), 0x123456);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kLittleEndian>().readU24(itr), 0x563412);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kBigEndian>().readU32(itr), 0x12345678);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kLittleEndian>().readU32(itr), 0x78563412);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kBigEndian>().readU64(itr), 0x123456789ABCDEF0LL);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kLittleEndian>().readU64(itr), 0xF0DEBC9A78563412LL);
  itr.rewind();
}

TEST(Read, SignedNegative) {
  byte data[] = {byte{0xFF}, byte{0xFE}, byte{0xFD}, byte{0xFC},
                 byte{0xFB}, byte{0xFA}, byte{0xF9}, byte{0xF8}};
  MultipassMemoryIterator itr(data, data + 8);

  EXPECT_EQ(ReadBeS16(itr), -1 - 0x0001);
  itr.rewind();
  EXPECT_EQ(ReadLeS16(itr), -1 - 0x0100);
  itr.rewind();
  EXPECT_EQ(ReadBeS24(itr), -1 - 0x000102);
  itr.rewind();
  EXPECT_EQ(ReadLeS24(itr), -1 - 0x020100);
  itr.rewind();
  EXPECT_EQ(ReadBeS32(itr), -1 - 0x00010203);
  itr.rewind();
  EXPECT_EQ(ReadLeS32(itr), -1 - 0x03020100);
  itr.rewind();
  EXPECT_EQ(ReadBeS64(itr), -1 - 0x0001020304050607);
  itr.rewind();
  EXPECT_EQ(ReadLeS64(itr), -1 - 0x0706050403020100);
  itr.rewind();
}

TEST(Read, SignedNegativeTemplated) {
  byte data[] = {byte{0xFF}, byte{0xFE}, byte{0xFD}, byte{0xFC},
                 byte{0xFB}, byte{0xFA}, byte{0xF9}, byte{0xF8}};
  MultipassMemoryIterator itr(data, data + 8);

  EXPECT_EQ(IntegerReader<kBigEndian>().readS16(itr), -1 - 0x0001);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kLittleEndian>().readS16(itr), -1 - 0x0100);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kBigEndian>().readS24(itr), -1 - 0x000102);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kLittleEndian>().readS24(itr), -1 - 0x020100);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kBigEndian>().readS32(itr), -1 - 0x00010203);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kLittleEndian>().readS32(itr), -1 - 0x03020100);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kBigEndian>().readS64(itr), -1 - 0x0001020304050607);
  itr.rewind();
  EXPECT_EQ(IntegerReader<kLittleEndian>().readS64(itr),
            -1 - 0x0706050403020100);
  itr.rewind();
}

TEST(Read, SimpleOverflow) {
  // Spot-checking only, because the code under test is simple and repetitive.
  byte data[] = {byte{0xFF}};
  MultipassMemoryIterator itr(data, data + 1);
  ASSERT_EQ(kOk, itr.status());
  ReadBeU16(itr);
  ASSERT_EQ(kEndOfStream, itr.status());
  itr.rewind();
  ASSERT_EQ(kOk, itr.status());
  ReadBeS32(itr);
  ASSERT_EQ(kEndOfStream, itr.status());
  itr.rewind();
  ASSERT_EQ(kOk, itr.status());
  ReadBeU64(itr);
  ASSERT_EQ(kEndOfStream, itr.status());
  itr.rewind();
}

TEST(Read, Float) {
  float num = 34664315.451;
  UnsafeMemoryIterator itr((const byte*)&num);
  EXPECT_EQ(num, HostNativeReader<float>().read(itr));
}

TEST(Read, Double) {
  double num = 34664315.451;
  UnsafeMemoryIterator itr((const byte*)&num);
  EXPECT_EQ(num, HostNativeReader<double>().read(itr));
}

TEST(Read, HostNativeOverflow) {
  double num = 34664315.451;
  MemoryIterator itr((const byte*)&num, (const byte*)&num + 2);
  EXPECT_EQ(5.0, HostNativeReader<double>().read(itr, 5.0));
  EXPECT_EQ(kEndOfStream, itr.status());
}

TEST(Read, VarU64_0) {
  byte d[] = {byte{0}};
  MemoryIterator i(d, d + 1);
  EXPECT_EQ(0, ReadVarU64(i));
}

TEST(Read, VarU64_1) {
  byte d[] = {byte{1}};
  MemoryIterator i(d, d + 1);
  EXPECT_EQ(1, ReadVarU64(i));
}

TEST(Read, VarU64_127) {
  byte d[] = {byte{0x7F}};
  MemoryIterator i(d, d + 1);
  EXPECT_EQ(127, ReadVarU64(i));
}

TEST(Read, VarU64_128) {
  byte d[] = {byte{0x80}, byte{0x01}};
  MemoryIterator i(d, d + 2);
  EXPECT_EQ(128, ReadVarU64(i));
}

TEST(Read, VarU64_150) {
  byte d[] = {byte{0x96}, byte{0x01}};
  MemoryIterator i(d, d + 2);
  EXPECT_EQ(150, ReadVarU64(i));
}

struct DrippingIterator {
  const char* data;
  const char* end;
  size_t read(byte* buf, size_t count) {
    if (count > 3) count = 3;
    if (count > end - data) count = end - data;
    memcpy(buf, data, count);
    data += count;
    return count;
  }
};

TEST(Read, ByteArray) {
  const char* in = "ABCDEFGH";
  DrippingIterator itr{in, in + 8};
  char result[] = "        ";
  EXPECT_EQ(5, ReadByteArray(itr, (byte*)result, 5));
  EXPECT_STREQ("ABCDE   ", result);
}

TEST(Read, ByteArrayOverflow) {
  const char* in = "ABCD";
  DrippingIterator itr{in, in + 4};
  char result[] = "        ";
  EXPECT_EQ(4, ReadByteArray(itr, (byte*)result, 10));
  EXPECT_STREQ("ABCD    ", result);
}

TEST(Read, EmptyCString) {
  const byte in[] = {byte{0}};
  MemoryIterator itr{in, in + 1};
  char buf[] = {9, 9, 9, 9, 9};
  EXPECT_EQ(0, ReadCString(itr, buf, 5));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_THAT(buf, ElementsAre(0, 9, 9, 9, 9));
}

TEST(Read, ShortCString) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}};
  MemoryIterator itr{in, in + 4};
  char buf[] = {9, 9, 9, 9, 9};
  EXPECT_EQ(3, ReadCString(itr, buf, 5));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_THAT(buf, ElementsAre('f', 'o', 'o', 0, 9));
}

TEST(Read, ShortCStringZeroBuf) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}, byte{7}};
  MemoryIterator itr{in, in + 5};
  char buf[] = {9, 9, 9, 9, 9};
  EXPECT_EQ(0, ReadCString(itr, buf, 0));
  EXPECT_EQ(7, ReadU8(itr));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_THAT(buf, ElementsAre(9, 9, 9, 9, 9));
}

TEST(Read, ShortCStringUnderBuf) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}, byte{7}};
  MemoryIterator itr{in, in + 5};
  char buf[] = {9, 9, 9, 9, 9};
  EXPECT_EQ(1, ReadCString(itr, buf, 2));
  EXPECT_EQ(7, ReadU8(itr));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_THAT(buf, ElementsAre('f', 0, 9, 9, 9));
}

TEST(Read, EmptyString) {
  const byte in[] = {byte{0}};
  MemoryIterator itr{in, in + 1};
  EXPECT_EQ("", ReadString(itr, 5));
  EXPECT_EQ(kOk, itr.status());
}

TEST(Read, ShortString) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}};
  MemoryIterator itr{in, in + 4};
  EXPECT_EQ("foo", ReadString(itr, 5));
  EXPECT_EQ(kOk, itr.status());
}

TEST(Read, ShortStringZeroBuf) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}, byte{7}};
  MemoryIterator itr{in, in + 5};
  EXPECT_EQ("", ReadString(itr, 0));
  EXPECT_EQ(7, ReadU8(itr));
  EXPECT_EQ(kOk, itr.status());
}

TEST(Read, ShortStringUnderBuf) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}, byte{7}};
  MemoryIterator itr{in, in + 5};
  EXPECT_EQ("f", ReadString(itr, 1));
  EXPECT_EQ(7, ReadU8(itr));
  EXPECT_EQ(kOk, itr.status());
}

TEST(Read, ShortStringView) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}};
  MemoryIterator itr{in, in + 4};
  EXPECT_EQ("foo", ReadStringView(itr));
  EXPECT_EQ(kOk, itr.status());
}

TEST(Read, ShortStringViewOverflow) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}};
  MemoryIterator itr{in, in + 3};
  EXPECT_EQ("fo", ReadStringView(itr));
  EXPECT_EQ(kEndOfStream, itr.status());
}

}  // namespace roo_io