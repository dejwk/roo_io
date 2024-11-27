#include "roo_io/data/read.h"

#include "gtest/gtest.h"
#include "roo_io/iterator/memory_input_iterator.h"

namespace roo_io {

TEST(Read, Unsigned) {
  byte data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
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
  byte data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
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
  byte data[] = {0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8};
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
  byte data[] = {0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8};
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


}  // namespace roo_io