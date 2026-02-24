#include "roo_io/data/multipass_input_stream_reader.h"

#include <stdint.h>

#include <cstring>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "roo_io/data/ieee754.h"
#include "roo_io/data/read.h"
#include "roo_io/memory/memory_input_stream.h"

using testing::ElementsAre;

namespace roo_io {

namespace {

float FloatFromBits(uint32_t bits) {
  float value;
  memcpy(&value, &bits, sizeof(value));
  return value;
}

double DoubleFromBits(uint64_t bits) {
  double value;
  memcpy(&value, &bits, sizeof(value));
  return value;
}

}  // namespace

MultipassInputStreamReader NewReader(const byte* begin, const byte* end) {
  return MultipassInputStreamReader(
      std::unique_ptr<MultipassInputStream>(new MemoryInputStream(begin, end)));
}

TEST(Reader, DefaultConstructible) {
  MultipassInputStreamReader reader;
  EXPECT_EQ(kClosed, reader.status());
  reader.readBeU16();
  EXPECT_EQ(kClosed, reader.status());
}

TEST(Reader, Unsigned) {
  byte data[] = {byte{0x12}, byte{0x34}, byte{0x56}, byte{0x78},
                 byte{0x9A}, byte{0xBC}, byte{0xDE}, byte{0xF0}};
  MultipassInputStreamReader reader;
  reader = NewReader(data, data + 8);
  EXPECT_EQ(reader.readBeU16(), 0x1234);
  reader.rewind();
  EXPECT_EQ(reader.readLeU16(), 0x3412);
  reader.rewind();
  EXPECT_EQ(reader.readBeU24(), 0x123456);
  reader.rewind();
  EXPECT_EQ(reader.readLeU24(), 0x563412);
  reader.rewind();
  EXPECT_EQ(reader.readBeU32(), 0x12345678);
  reader.rewind();
  EXPECT_EQ(reader.readLeU32(), 0x78563412);
  reader.rewind();
  EXPECT_EQ(reader.readBeU64(), 0x123456789ABCDEF0LL);
  reader.rewind();
  EXPECT_EQ(reader.readLeU64(), 0xF0DEBC9A78563412LL);
}

TEST(Reader, SignedNegative) {
  byte data[] = {byte{0xFF}, byte{0xFE}, byte{0xFD}, byte{0xFC},
                 byte{0xFB}, byte{0xFA}, byte{0xF9}, byte{0xF8}};
  MultipassInputStreamReader reader;
  reader = NewReader(data, data + 8);
  EXPECT_EQ(reader.readBeS16(), -1 - 0x0001);
  reader.rewind();
  EXPECT_EQ(reader.readLeS16(), -1 - 0x0100);
  reader.rewind();
  EXPECT_EQ(reader.readBeS24(), -1 - 0x000102);
  reader.rewind();
  EXPECT_EQ(reader.readLeS24(), -1 - 0x020100);
  reader.rewind();
  EXPECT_EQ(reader.readBeS32(), -1 - 0x00010203);
  reader.rewind();
  EXPECT_EQ(reader.readLeS32(), -1 - 0x03020100);
  reader.rewind();
  EXPECT_EQ(reader.readBeS64(), -1 - 0x0001020304050607);
  reader.rewind();
  EXPECT_EQ(reader.readLeS64(), -1 - 0x0706050403020100);
  reader.rewind();
}

TEST(Reader, Float) {
  float num = 34664315.451;
  MultipassInputStreamReader reader =
      NewReader((const byte*)&num, ((const byte*)&num) + 4);
  EXPECT_EQ(num, reader.readHostNative<float>());
}

TEST(Reader, Double) {
  double num = 34664315.451;
  MultipassInputStreamReader reader =
      NewReader((const byte*)&num, ((const byte*)&num) + 8);
  EXPECT_EQ(num, reader.readHostNative<double>());
}

#if ROO_IO_IEEE754
TEST(Reader, FloatEndian) {
  const float expected = FloatFromBits(0x3F800000u);
  byte be[] = {byte{0x3F}, byte{0x80}, byte{0x00}, byte{0x00}};
  byte le[] = {byte{0x00}, byte{0x00}, byte{0x80}, byte{0x3F}};

  MultipassInputStreamReader be_reader = NewReader(be, be + 4);
  EXPECT_EQ(expected, be_reader.readBeFloat());

  MultipassInputStreamReader le_reader = NewReader(le, le + 4);
  EXPECT_EQ(expected, le_reader.readLeFloat());
}

TEST(Reader, DoubleEndian) {
  const double expected = DoubleFromBits(0x3FF0000000000000ULL);
  byte be[] = {byte{0x3F}, byte{0xF0}, byte{0x00}, byte{0x00},
               byte{0x00}, byte{0x00}, byte{0x00}, byte{0x00}};
  byte le[] = {byte{0x00}, byte{0x00}, byte{0x00}, byte{0x00},
               byte{0x00}, byte{0x00}, byte{0xF0}, byte{0x3F}};

  MultipassInputStreamReader be_reader = NewReader(be, be + 8);
  EXPECT_EQ(expected, be_reader.readBeDouble());

  MultipassInputStreamReader le_reader = NewReader(le, le + 8);
  EXPECT_EQ(expected, le_reader.readLeDouble());
}
#endif  // ROO_IO_IEEE754

TEST(Reader, HostNativeOverflow) {
  double num = 34664315.451;
  MultipassInputStreamReader reader =
      NewReader((const byte*)&num, ((const byte*)&num) + 2);
  EXPECT_EQ(5.0, reader.readHostNative<double>(5.0));
  EXPECT_EQ(kEndOfStream, reader.status());
}

TEST(Reader, VarU64_150) {
  byte d[] = {byte{0x96}, byte{0x01}};
  MultipassInputStreamReader reader = NewReader(d, d + 2);
  EXPECT_EQ(150, reader.readVarU64());
}

TEST(Reader, ByteArray) {
  const byte* in = (const byte*)"ABCDEFGH";
  MultipassInputStreamReader reader = NewReader(in, in + 8);
  char result[] = "        ";
  EXPECT_EQ(5, reader.readByteArray((byte*)result, 5));
  EXPECT_STREQ("ABCDE   ", result);
}

TEST(Read, ShortCString) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}};
  MultipassInputStreamReader reader = NewReader(in, in + 4);
  char buf[] = {9, 9, 9, 9, 9};
  EXPECT_EQ(3, reader.readCString(buf, 5));
  EXPECT_EQ(kOk, reader.status());
  EXPECT_THAT(buf, ElementsAre('f', 'o', 'o', 0, 9));
}

TEST(Read, ShortCStringUnderBuf) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}, byte{7}};
  MultipassInputStreamReader reader = NewReader(in, in + 5);
  char buf[] = {9, 9, 9, 9, 9};
  EXPECT_EQ(1, reader.readCString(buf, 2));
  EXPECT_EQ(7, reader.readU8());
  EXPECT_EQ(kOk, reader.status());
  EXPECT_THAT(buf, ElementsAre('f', 0, 9, 9, 9));
}

TEST(Read, ShortString) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}};
  MultipassInputStreamReader reader = NewReader(in, in + 4);
  EXPECT_EQ("foo", reader.readString(5));
  EXPECT_EQ(kOk, reader.status());
}

TEST(Read, ShortStringUnderBuf) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}, byte{7}};
  MultipassInputStreamReader reader = NewReader(in, in + 5);
  EXPECT_EQ("f", reader.readString(1));
  EXPECT_EQ(7, reader.readU8());
  EXPECT_EQ(kOk, reader.status());
}

}  // namespace roo_io