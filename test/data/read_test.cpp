#include "roo_io/data/read.h"

#include <stdint.h>

#include <cstring>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "roo_io/data/ieee754.h"
#include "roo_io/memory/memory_input_iterator.h"

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

#if ROO_IO_IEEE754
TEST(Read, FloatEndian) {
  const uint32_t bits = 0x3F800000u;
  const float expected = FloatFromBits(bits);
  byte be[] = {byte{0x3F}, byte{0x80}, byte{0x00}, byte{0x00}};
  byte le[] = {byte{0x00}, byte{0x00}, byte{0x80}, byte{0x3F}};

  MultipassMemoryIterator be_itr(be, be + 4);
  EXPECT_EQ(expected, ReadBeFloat(be_itr));
  be_itr.rewind();
  const float be_value = ReadFloat<MultipassMemoryIterator, kBigEndian>(be_itr);
  EXPECT_EQ(expected, be_value);

  MultipassMemoryIterator le_itr(le, le + 4);
  EXPECT_EQ(expected, ReadLeFloat(le_itr));
  le_itr.rewind();
  const float le_value =
      ReadFloat<MultipassMemoryIterator, kLittleEndian>(le_itr);
  EXPECT_EQ(expected, le_value);
}

TEST(Read, DoubleEndian) {
  const uint64_t bits = 0x3FF0000000000000ULL;
  const double expected = DoubleFromBits(bits);
  byte be[] = {byte{0x3F}, byte{0xF0}, byte{0x00}, byte{0x00},
               byte{0x00}, byte{0x00}, byte{0x00}, byte{0x00}};
  byte le[] = {byte{0x00}, byte{0x00}, byte{0x00}, byte{0x00},
               byte{0x00}, byte{0x00}, byte{0xF0}, byte{0x3F}};

  MultipassMemoryIterator be_itr(be, be + 8);
  EXPECT_EQ(expected, ReadBeDouble(be_itr));
  be_itr.rewind();
  const double be_value =
      ReadDouble<MultipassMemoryIterator, kBigEndian>(be_itr);
  EXPECT_EQ(expected, be_value);

  MultipassMemoryIterator le_itr(le, le + 8);
  EXPECT_EQ(expected, ReadLeDouble(le_itr));
  le_itr.rewind();
  const double le_value =
      ReadDouble<MultipassMemoryIterator, kLittleEndian>(le_itr);
  EXPECT_EQ(expected, le_value);
}
#endif  // ROO_IO_IEEE754

TEST(Read, HostNativeOverflow) {
  double num = 34664315.451;
  MemoryIterator itr((const byte*)&num, (const byte*)&num + 2);
  EXPECT_EQ(5.0, HostNativeReader<double>().read(itr, 5.0));
  EXPECT_EQ(kEndOfStream, itr.status());
}

TEST(Read, VarU64_0) {
  byte d[] = {byte{0}};
  MemoryIterator i(d, d + 1);
  uint64_t value = 0;
  EXPECT_TRUE(ReadVarU64(i, value));
  EXPECT_EQ(0, value);
}

TEST(Read, VarU64_1) {
  byte d[] = {byte{1}};
  MemoryIterator i(d, d + 1);
  uint64_t value = 0;
  EXPECT_TRUE(ReadVarU64(i, value));
  EXPECT_EQ(1, value);
}

TEST(Read, VarU64_127) {
  byte d[] = {byte{0x7F}};
  MemoryIterator i(d, d + 1);
  uint64_t value = 0;
  EXPECT_TRUE(ReadVarU64(i, value));
  EXPECT_EQ(127, value);
}

TEST(Read, VarU64_128) {
  byte d[] = {byte{0x80}, byte{0x01}};
  MemoryIterator i(d, d + 2);
  uint64_t value = 0;
  EXPECT_TRUE(ReadVarU64(i, value));
  EXPECT_EQ(128, value);
}

TEST(Read, VarU64_150) {
  byte d[] = {byte{0x96}, byte{0x01}};
  MemoryIterator i(d, d + 2);
  uint64_t value = 0;
  EXPECT_TRUE(ReadVarU64(i, value));
  EXPECT_EQ(150, value);
}

// Verifies checked decoding accepts the full uint64_t range and non-minimal
// encodings without changing the output until a complete value is available.
TEST(Read, CheckedVarU64AcceptsRepresentableEncodings) {
  byte max_value[] = {byte{0xFF}, byte{0xFF}, byte{0xFF}, byte{0xFF},
                      byte{0xFF}, byte{0xFF}, byte{0xFF}, byte{0xFF},
                      byte{0xFF}, byte{0x01}};
  MemoryIterator max_input(max_value, max_value + 10);
  uint64_t value = 0;
  EXPECT_TRUE(ReadVarU64(max_input, value));
  EXPECT_EQ(std::numeric_limits<uint64_t>::max(), value);
  EXPECT_EQ(kOk, max_input.status());

  byte non_minimal_zero[] = {byte{0x80}, byte{0x00}};
  MemoryIterator non_minimal_input(non_minimal_zero, non_minimal_zero + 2);
  value = 17;
  EXPECT_TRUE(ReadVarU64(non_minimal_input, value));
  EXPECT_EQ(0, value);
}

// Verifies malformed byte ten is reported without consuming the next record.
TEST(Read, CheckedVarU64RejectsInvalidTenthByte) {
  byte data[] = {byte{0x80}, byte{0x80}, byte{0x80}, byte{0x80},
                 byte{0x80}, byte{0x80}, byte{0x80}, byte{0x80},
                 byte{0x80}, byte{0x82}, byte{0x7B}};
  MemoryIterator input(data, data + 11);
  uint64_t value = 99;
  EXPECT_FALSE(ReadVarU64(input, value));
  EXPECT_EQ(99, value);
  EXPECT_EQ(kOk, input.status());
  EXPECT_EQ(byte{0x7B}, input.read());
}

// Verifies truncated prefixes preserve the caller's value and report EOF.
TEST(Read, CheckedVarU64RejectsTruncatedPrefix) {
  byte data[] = {byte{0x80}, byte{0x80}};
  MemoryIterator input(data, data + 2);
  uint64_t value = 99;
  EXPECT_FALSE(ReadVarU64(input, value));
  EXPECT_EQ(99, value);
  EXPECT_EQ(kEndOfStream, input.status());
}

// Verifies ZigZag transforms round-trip signed boundary values without signed
// overflow or an arithmetic right shift.
TEST(Read, ZigZagTransforms) {
  EXPECT_EQ(0U, ZigZagEncode32(0));
  EXPECT_EQ(1U, ZigZagEncode32(-1));
  EXPECT_EQ(2U, ZigZagEncode32(1));
  EXPECT_EQ(std::numeric_limits<uint32_t>::max(),
            ZigZagEncode32(std::numeric_limits<int32_t>::min()));
  EXPECT_EQ(std::numeric_limits<int64_t>::min(),
            ZigZagDecode64(std::numeric_limits<uint64_t>::max()));
  EXPECT_EQ(std::numeric_limits<int64_t>::max(),
            ZigZagDecode64(std::numeric_limits<uint64_t>::max() - 1));
}

// Verifies checked ZigZag reads preserve output on malformed narrower values.
TEST(Read, ZigZagReads) {
  const byte encoded[] = {byte{0x01}, byte{0x02}};
  MemoryIterator input(encoded, encoded + 2);
  int32_t value32 = 0;
  EXPECT_TRUE(ReadZigZag32(input, value32));
  EXPECT_EQ(-1, value32);
  int64_t value64 = 0;
  EXPECT_TRUE(ReadZigZag64(input, value64));
  EXPECT_EQ(1, value64);

  const byte overflow[] = {byte{0x80}, byte{0x80}, byte{0x80}, byte{0x80},
                           byte{0x10}};
  MemoryIterator overflow_input(overflow, overflow + 5);
  value32 = 99;
  EXPECT_FALSE(ReadZigZag32(overflow_input, value32));
  EXPECT_EQ(99, value32);
  EXPECT_EQ(kOk, overflow_input.status());
}

// Verifies narrower unsigned varints reject high bits without changing output.
TEST(Read, VarU32RejectsOverflow) {
  const byte overflow[] = {byte{0x80}, byte{0x80}, byte{0x80}, byte{0x80},
                           byte{0x10}};
  MemoryIterator input(overflow, overflow + 5);
  uint32_t value = 99;
  EXPECT_FALSE(ReadVarU32(input, value));
  EXPECT_EQ(99U, value);
  EXPECT_EQ(kOk, input.status());
}

struct DrippingIterator {
  const char* data;
  const char* end;
  size_t read(byte* buf, size_t count) {
    if (count > 3) count = 3;
    const size_t available = static_cast<size_t>(end - data);
    if (count > available) count = available;
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
  size_t len = 9;
  EXPECT_TRUE(ReadCString(itr, buf, 5, &len));
  EXPECT_EQ(0, len);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_THAT(buf, ElementsAre(0, 9, 9, 9, 9));
}

TEST(Read, ShortCString) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}};
  MemoryIterator itr{in, in + 4};
  char buf[] = {9, 9, 9, 9, 9};
  size_t len = 0;
  EXPECT_TRUE(ReadCString(itr, buf, 5, &len));
  EXPECT_EQ(3, len);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_THAT(buf, ElementsAre('f', 'o', 'o', 0, 9));
}

TEST(Read, ShortCStringZeroBuf) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}, byte{7}};
  MemoryIterator itr{in, in + 5};
  char buf[] = {9, 9, 9, 9, 9};
  size_t len = 0;
  EXPECT_FALSE(ReadCString(itr, buf, 0, &len));
  EXPECT_EQ(3, ReadU8(itr));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_THAT(buf, ElementsAre(9, 9, 9, 9, 9));
}

TEST(Read, ShortCStringUnderBuf) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}, byte{7}};
  MemoryIterator itr{in, in + 5};
  char buf[] = {9, 9, 9, 9, 9};
  size_t len = 0;
  EXPECT_TRUE(ReadCStringTruncated(itr, buf, 2, &len));
  EXPECT_EQ(1, len);
  EXPECT_EQ(7, ReadU8(itr));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_THAT(buf, ElementsAre('f', 0, 9, 9, 9));
}

// Verifies strict size rejection leaves the payload available and truncated
// reads consume the complete payload before the following field.
TEST(Read, StringReadPolicies) {
  const byte input[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}, byte{7}};
  char buf[] = {9, 9, 9, 9, 9};
  size_t len = 0;
  MemoryIterator strict_cstring(input, input + 5);
  EXPECT_FALSE(ReadCString(strict_cstring, buf, 2, &len));
  EXPECT_EQ(kOk, strict_cstring.status());
  EXPECT_EQ(byte{'f'}, strict_cstring.read());

  std::string result;
  MemoryIterator strict_string(input, input + 5);
  EXPECT_FALSE(ReadString(strict_string, &result, 2));
  EXPECT_EQ(byte{'f'}, strict_string.read());

  MemoryIterator truncated_string(input, input + 5);
  EXPECT_TRUE(ReadStringTruncated(truncated_string, &result, 1));
  EXPECT_EQ("f", result);
  EXPECT_EQ(byte{7}, truncated_string.read());
}

TEST(Read, EmptyString) {
  const byte in[] = {byte{0}};
  MemoryIterator itr{in, in + 1};
  std::string result;
  EXPECT_TRUE(ReadString(itr, &result, 5));
  EXPECT_EQ("", result);
  EXPECT_EQ(kOk, itr.status());
}

TEST(Read, ShortString) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}};
  MemoryIterator itr{in, in + 4};
  std::string result;
  EXPECT_TRUE(ReadString(itr, &result, 5));
  EXPECT_EQ("foo", result);
  EXPECT_EQ(kOk, itr.status());
}

TEST(Read, ShortStringZeroBuf) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}, byte{7}};
  MemoryIterator itr{in, in + 5};
  std::string result;
  EXPECT_TRUE(ReadStringTruncated(itr, &result, 0));
  EXPECT_EQ("", result);
  EXPECT_EQ(7, ReadU8(itr));
  EXPECT_EQ(kOk, itr.status());
}

TEST(Read, ShortStringUnderBuf) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}, byte{7}};
  MemoryIterator itr{in, in + 5};
  std::string result;
  EXPECT_TRUE(ReadStringTruncated(itr, &result, 1));
  EXPECT_EQ("f", result);
  EXPECT_EQ(7, ReadU8(itr));
  EXPECT_EQ(kOk, itr.status());
}

TEST(Read, ShortStringView) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}, byte{'o'}};
  MemoryIterator itr{in, in + 4};
  roo::string_view result;
  EXPECT_TRUE(ReadStringView(itr, &result));
  EXPECT_EQ("foo", result);
  EXPECT_EQ(kOk, itr.status());
}

TEST(Read, ShortStringViewOverflow) {
  const byte in[] = {byte{3}, byte{'f'}, byte{'o'}};
  MemoryIterator itr{in, in + 3};
  roo::string_view result;
  EXPECT_FALSE(ReadStringView(itr, &result));
  EXPECT_EQ(kEndOfStream, itr.status());
}

}  // namespace roo_io
