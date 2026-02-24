#include "roo_io/memory/load.h"

#include <stdint.h>

#include <cstring>

#include "gtest/gtest.h"
#include "roo_io/data/ieee754.h"

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

const byte data[] = {byte{0x12}, byte{0x34}, byte{0x56}, byte{0x78},
                     byte{0x9A}, byte{0xBC}, byte{0xDE}, byte{0xF0}};

TEST(Load, Unsigned) {
  EXPECT_EQ(LoadU8(data), 0x12);
  EXPECT_EQ(LoadBeU16(data), 0x1234);
  EXPECT_EQ(LoadLeU16(data), 0x3412);
  EXPECT_EQ(LoadBeU24(data), 0x123456);
  EXPECT_EQ(LoadLeU24(data), 0x563412);
  EXPECT_EQ(LoadBeU32(data), 0x12345678);
  EXPECT_EQ(LoadLeU32(data), 0x78563412);
  EXPECT_EQ(LoadBeU64(data), 0x123456789ABCDEF0LL);
  EXPECT_EQ(LoadLeU64(data), 0xF0DEBC9A78563412LL);
}

TEST(Load, UnsignedTemplated) {
  EXPECT_EQ(LoadU16<kBigEndian>(data), 0x1234);
  EXPECT_EQ(LoadU16<kLittleEndian>(data), 0x3412);
  EXPECT_EQ(LoadU24<kBigEndian>(data), 0x123456);
  EXPECT_EQ(LoadU24<kLittleEndian>(data), 0x563412);
  EXPECT_EQ(LoadU32<kBigEndian>(data), 0x12345678);
  EXPECT_EQ(LoadU32<kLittleEndian>(data), 0x78563412);
  EXPECT_EQ(LoadU64<kBigEndian>(data), 0x123456789ABCDEF0LL);
  EXPECT_EQ(LoadU64<kLittleEndian>(data), 0xF0DEBC9A78563412LL);

  EXPECT_EQ(LoadBeInteger<uint8_t>(data), 0x12);
  EXPECT_EQ(LoadLeInteger<uint8_t>(data), 0x12);
  EXPECT_EQ(LoadBeInteger<uint16_t>(data), 0x1234);
  EXPECT_EQ(LoadLeInteger<uint16_t>(data), 0x3412);
  EXPECT_EQ(LoadBeInteger<uint32_t>(data), 0x12345678);
  EXPECT_EQ(LoadLeInteger<uint32_t>(data), 0x78563412);
  EXPECT_EQ(LoadBeInteger<uint64_t>(data), 0x123456789ABCDEF0LL);
  EXPECT_EQ(LoadLeInteger<uint64_t>(data), 0xF0DEBC9A78563412LL);

  EXPECT_EQ((LoadInteger<kBigEndian, uint8_t>(data)), 0x12);
  EXPECT_EQ((LoadInteger<kLittleEndian, uint8_t>(data)), 0x12);
  EXPECT_EQ((LoadInteger<kBigEndian, uint16_t>(data)), 0x1234);
  EXPECT_EQ((LoadInteger<kLittleEndian, uint16_t>(data)), 0x3412);
  EXPECT_EQ((LoadInteger<kBigEndian, uint32_t>(data)), 0x12345678);
  EXPECT_EQ((LoadInteger<kLittleEndian, uint32_t>(data)), 0x78563412);
  EXPECT_EQ((LoadInteger<kBigEndian, uint64_t>(data)), 0x123456789ABCDEF0LL);
  EXPECT_EQ((LoadInteger<kLittleEndian, uint64_t>(data)), 0xF0DEBC9A78563412LL);
}

TEST(Load, SignedSimple) {
  EXPECT_EQ(LoadS8(data), 0x12);
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
  const byte data[] = {byte{0xFF}, byte{0xFE}, byte{0xFD}, byte{0xFC},
                       byte{0xFB}, byte{0xFA}, byte{0xF9}, byte{0xF8}};

  EXPECT_EQ(LoadS8(data), -1);
  EXPECT_EQ(LoadBeS16(data), -1 - 0x0001);
  EXPECT_EQ(LoadLeS16(data), -1 - 0x0100);
  EXPECT_EQ(LoadBeS24(data), -1 - 0x000102);
  EXPECT_EQ(LoadLeS24(data), -1 - 0x020100);
  EXPECT_EQ(LoadBeS32(data), -1 - 0x00010203);
  EXPECT_EQ(LoadLeS32(data), -1 - 0x03020100);
  EXPECT_EQ(LoadBeS64(data), -1 - 0x0001020304050607);
  EXPECT_EQ(LoadLeS64(data), -1 - 0x0706050403020100);
}

TEST(Load, SignedNegativeTemplated) {
  const byte data[] = {byte{0xFF}, byte{0xFE}, byte{0xFD}, byte{0xFC},
                       byte{0xFB}, byte{0xFA}, byte{0xF9}, byte{0xF8}};

  EXPECT_EQ(LoadS16<kBigEndian>(data), -1 - 0x0001);
  EXPECT_EQ(LoadS16<kLittleEndian>(data), -1 - 0x0100);
  EXPECT_EQ(LoadS24<kBigEndian>(data), -1 - 0x000102);
  EXPECT_EQ(LoadS24<kLittleEndian>(data), -1 - 0x020100);
  EXPECT_EQ(LoadS32<kBigEndian>(data), -1 - 0x00010203);
  EXPECT_EQ(LoadS32<kLittleEndian>(data), -1 - 0x03020100);
  EXPECT_EQ(LoadS64<kBigEndian>(data), -1 - 0x0001020304050607);
  EXPECT_EQ(LoadS64<kLittleEndian>(data), -1 - 0x0706050403020100);

  EXPECT_EQ(LoadBeInteger<int8_t>(data), -1);
  EXPECT_EQ(LoadLeInteger<int8_t>(data), -1);
  EXPECT_EQ(LoadBeInteger<int16_t>(data), -1 - 0x0001);
  EXPECT_EQ(LoadLeInteger<int16_t>(data), -1 - 0x0100);
  EXPECT_EQ(LoadBeInteger<int32_t>(data), -1 - 0x00010203);
  EXPECT_EQ(LoadLeInteger<int32_t>(data), -1 - 0x03020100);
  EXPECT_EQ(LoadBeInteger<int64_t>(data), -1 - 0x0001020304050607);
  EXPECT_EQ(LoadLeInteger<int64_t>(data), -1 - 0x0706050403020100);

  EXPECT_EQ((LoadInteger<kBigEndian, int8_t>(data)), -1);
  EXPECT_EQ((LoadInteger<kLittleEndian, int8_t>(data)), -1);
  EXPECT_EQ((LoadInteger<kBigEndian, int16_t>(data)), -1 - 0x0001);
  EXPECT_EQ((LoadInteger<kLittleEndian, int16_t>(data)), -1 - 0x0100);
  EXPECT_EQ((LoadInteger<kBigEndian, int32_t>(data)), -1 - 0x00010203);
  EXPECT_EQ((LoadInteger<kLittleEndian, int32_t>(data)), -1 - 0x03020100);
  EXPECT_EQ((LoadInteger<kBigEndian, int64_t>(data)), -1 - 0x0001020304050607);
  EXPECT_EQ((LoadInteger<kLittleEndian, int64_t>(data)),
            -1 - 0x0706050403020100);
}

TEST(Load, Float) {
  float num = 34664315.451;
  EXPECT_EQ(num, LoadHostNative<float>((const byte*)&num));
}

TEST(Load, Double) {
  double num = 34664315.451;
  EXPECT_EQ(num, LoadHostNative<double>((const byte*)&num));
}

#if ROO_IO_IEEE754
TEST(Load, FloatEndian) {
  const uint32_t bits = 0x3F800000u;
  const float expected = FloatFromBits(bits);
  const byte be[] = {byte{0x3F}, byte{0x80}, byte{0x00}, byte{0x00}};
  const byte le[] = {byte{0x00}, byte{0x00}, byte{0x80}, byte{0x3F}};

  EXPECT_EQ(expected, LoadBeFloat(be));
  EXPECT_EQ(expected, LoadLeFloat(le));
  EXPECT_EQ(expected, LoadFloat<kBigEndian>(be));
  EXPECT_EQ(expected, LoadFloat<kLittleEndian>(le));
}

TEST(Load, DoubleEndian) {
  const uint64_t bits = 0x3FF0000000000000ULL;
  const double expected = DoubleFromBits(bits);
  const byte be[] = {byte{0x3F}, byte{0xF0}, byte{0x00}, byte{0x00},
                     byte{0x00}, byte{0x00}, byte{0x00}, byte{0x00}};
  const byte le[] = {byte{0x00}, byte{0x00}, byte{0x00}, byte{0x00},
                     byte{0x00}, byte{0x00}, byte{0xF0}, byte{0x3F}};

  EXPECT_EQ(expected, LoadBeDouble(be));
  EXPECT_EQ(expected, LoadLeDouble(le));
  EXPECT_EQ(expected, LoadDouble<kBigEndian>(be));
  EXPECT_EQ(expected, LoadDouble<kLittleEndian>(le));
}
#endif  // ROO_IO_IEEE754

}  // namespace roo_io