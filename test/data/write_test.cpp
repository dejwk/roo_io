#include "roo_io/data/write.h"

#include <WString.h>
#include <stdint.h>

#include <cstring>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "roo_io/data/ieee754.h"
#include "roo_io/data/read.h"
#include "roo_io/memory/load.h"  // ForHostNativeWriter.
#include "roo_io/memory/memory_input_iterator.h"
#include "roo_io/memory/memory_output_iterator.h"

using namespace testing;

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

TEST(Write, BeU16) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteBeU16(itr, 0x4849);
  EXPECT_THAT(data, ElementsAre(0x48, 0x49, 9, 9, 9, 9, 9, 9));
}

TEST(Write, LeU16) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteLeU16(itr, 0x4849);
  EXPECT_THAT(data, ElementsAre(0x49, 0x48, 9, 9, 9, 9, 9, 9));
}

TEST(Write, BeU24) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteBeU24(itr, 0x48494A);
  EXPECT_THAT(data, ElementsAre(0x48, 0x49, 0x4A, 9, 9, 9, 9, 9));
}

TEST(Write, LeU24) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteLeU24(itr, 0x48494A);
  EXPECT_THAT(data, ElementsAre(0x4A, 0x49, 0x48, 9, 9, 9, 9, 9));
}

TEST(Write, BeU32) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteBeU32(itr, 0x48494A4B);
  EXPECT_THAT(data, ElementsAre(0x48, 0x49, 0x4A, 0x4B, 9, 9, 9, 9));
}

TEST(Write, LeU32) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteLeU32(itr, 0x48494A4B);
  EXPECT_THAT(data, ElementsAre(0x4B, 0x4A, 0x49, 0x48, 9, 9, 9, 9));
}

TEST(Write, BeU64) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 10);
  WriteBeU64(itr, 0x48494A4B4C4D4E4F);
  EXPECT_THAT(
      data, ElementsAre(0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 9, 9));
}

TEST(Write, LeU64) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 10);
  WriteLeU64(itr, 0x48494A4B4C4D4E4F);
  EXPECT_THAT(
      data, ElementsAre(0x4F, 0x4E, 0x4D, 0x4C, 0x4B, 0x4A, 0x49, 0x48, 9, 9));
}

TEST(Write, BeS16) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteBeS16(itr, -2);
  EXPECT_THAT(data, ElementsAre(0xFF, 0xFE, 9, 9, 9, 9, 9, 9));
}

TEST(Write, LeS16) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteLeS16(itr, -2);
  EXPECT_THAT(data, ElementsAre(0xFE, 0xFF, 9, 9, 9, 9, 9, 9));
}

TEST(Write, BeS24) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteBeS24(itr, -2);
  EXPECT_THAT(data, ElementsAre(0xFF, 0xFF, 0xFE, 9, 9, 9, 9, 9));
}

TEST(Write, LeS24) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteLeS24(itr, -2);
  EXPECT_THAT(data, ElementsAre(0xFE, 0xFF, 0xFF, 9, 9, 9, 9, 9));
}

TEST(Write, BeS32) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteBeS32(itr, -2);
  EXPECT_THAT(data, ElementsAre(0xFF, 0xFF, 0xFF, 0xFE, 9, 9, 9, 9));
}

TEST(Write, LeS32) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteLeS32(itr, -2);
  EXPECT_THAT(data, ElementsAre(0xFE, 0xFF, 0xFF, 0xFF, 9, 9, 9, 9));
}

TEST(Write, BeS64) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 10);
  WriteBeS64(itr, -2);
  EXPECT_THAT(
      data, ElementsAre(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 9, 9));
}

TEST(Write, LeS64) {
  uint8_t data[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 10);
  WriteLeS64(itr, -2);
  EXPECT_THAT(
      data, ElementsAre(0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 9, 9));
}

#if ROO_IO_IEEE754
TEST(Write, FloatEndian) {
  const float v = FloatFromBits(0x3F800000u);
  uint8_t data_be[] = {9, 9, 9, 9};
  uint8_t data_le[] = {9, 9, 9, 9};

  MemoryOutputIterator itr_be((byte*)data_be, (byte*)data_be + 4);
  WriteBeFloat(itr_be, v);
  EXPECT_THAT(data_be, ElementsAre(0x3F, 0x80, 0x00, 0x00));

  MemoryOutputIterator itr_le((byte*)data_le, (byte*)data_le + 4);
  WriteLeFloat(itr_le, v);
  EXPECT_THAT(data_le, ElementsAre(0x00, 0x00, 0x80, 0x3F));
}

TEST(Write, DoubleEndian) {
  const double v = DoubleFromBits(0x3FF0000000000000ULL);
  uint8_t data_be[] = {9, 9, 9, 9, 9, 9, 9, 9};
  uint8_t data_le[] = {9, 9, 9, 9, 9, 9, 9, 9};

  MemoryOutputIterator itr_be((byte*)data_be, (byte*)data_be + 8);
  WriteBeDouble(itr_be, v);
  EXPECT_THAT(data_be,
              ElementsAre(0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00));

  MemoryOutputIterator itr_le((byte*)data_le, (byte*)data_le + 8);
  WriteLeDouble(itr_le, v);
  EXPECT_THAT(data_le,
              ElementsAre(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x3F));
}

TEST(Write, FloatTemplated) {
  const float v = FloatFromBits(0x3F800000u);
  uint8_t data_be[] = {9, 9, 9, 9};
  uint8_t data_le[] = {9, 9, 9, 9};

  MemoryOutputIterator itr_be((byte*)data_be, (byte*)data_be + 4);
  WriteFloat<MemoryOutputIterator, kBigEndian>(itr_be, v);
  EXPECT_THAT(data_be, ElementsAre(0x3F, 0x80, 0x00, 0x00));

  MemoryOutputIterator itr_le((byte*)data_le, (byte*)data_le + 4);
  WriteFloat<MemoryOutputIterator, kLittleEndian>(itr_le, v);
  EXPECT_THAT(data_le, ElementsAre(0x00, 0x00, 0x80, 0x3F));
}

TEST(Write, DoubleTemplated) {
  const double v = DoubleFromBits(0x3FF0000000000000ULL);
  uint8_t data_be[] = {9, 9, 9, 9, 9, 9, 9, 9};
  uint8_t data_le[] = {9, 9, 9, 9, 9, 9, 9, 9};

  MemoryOutputIterator itr_be((byte*)data_be, (byte*)data_be + 8);
  WriteDouble<MemoryOutputIterator, kBigEndian>(itr_be, v);
  EXPECT_THAT(data_be,
              ElementsAre(0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00));

  MemoryOutputIterator itr_le((byte*)data_le, (byte*)data_le + 8);
  WriteDouble<MemoryOutputIterator, kLittleEndian>(itr_le, v);
  EXPECT_THAT(data_le,
              ElementsAre(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x3F));
}
#endif  // ROO_IO_IEEE754

TEST(Write, Float) {
  float v = 3.14159265;
  uint8_t data[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  HostNativeWriter<float>().write(itr, v);
  float r = LoadHostNative<float>((byte*)data);
  EXPECT_EQ(r, v);
  EXPECT_EQ(0, data[5]);
}

TEST(Write, VarU64_0) {
  uint8_t data[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteVarU64(itr, 0);
  EXPECT_THAT(data, ElementsAre(0, 0, 0, 0, 0, 0, 0, 0));
}

TEST(Write, VarU64_1) {
  uint8_t data[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteVarU64(itr, 1);
  EXPECT_THAT(data, ElementsAre(1, 0, 0, 0, 0, 0, 0, 0));
}

TEST(Write, VarU64_127) {
  uint8_t data[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteVarU64(itr, 127);
  EXPECT_THAT(data, ElementsAre(0x7F, 0, 0, 0, 0, 0, 0, 0));
}

TEST(Write, VarU64_128) {
  uint8_t data[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteVarU64(itr, 128);
  EXPECT_THAT(data, ElementsAre(0x80, 0x01, 0, 0, 0, 0, 0, 0));
}

TEST(Write, VarU64_150) {
  uint8_t data[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr((byte*)data, (byte*)data + 8);
  WriteVarU64(itr, 150);
  EXPECT_THAT(data, ElementsAre(0x96, 0x01, 0, 0, 0, 0, 0, 0));
}

// Verifies negative one uses the ZigZag varint golden encoding.
TEST(Write, ZigZagNegativeOne) {
  uint8_t data[] = {0, 0, 0, 0};
  MemoryOutputIterator iterator(reinterpret_cast<byte*>(data),
                                reinterpret_cast<byte*>(data) + 4);
  WriteZigZag32(iterator, -1);
  EXPECT_THAT(data, ElementsAre(0x01, 0, 0, 0));
}

TEST(Write, VarU32) {
  uint8_t data[] = {0, 0, 0, 0};
  MemoryOutputIterator iterator(reinterpret_cast<byte*>(data),
                                reinterpret_cast<byte*>(data) + 4);
  WriteVarU32(iterator, 150);
  EXPECT_THAT(data, ElementsAre(0x96, 0x01, 0, 0));
}

namespace {

struct ShortVarintSink {
  void write(byte value) { write(&value, 1); }
  size_t write(const byte* source, size_t count) {
    if (state != kOk) return 0;
    if (size == limit) {
      state = kNoSpaceLeftOnDevice;
      return 0;
    }
    if (count == 0) return 0;
    data[size++] = *source;
    return 1;
  }
  Status status() const { return state; }
  byte data[10] = {};
  size_t size = 0;
  size_t limit = 10;
  Status state = kOk;
};

// Checks both directions against a simple independent 64-bit reference encoder.
void CheckVarint(uint64_t value) {
  SCOPED_TRACE(value);
  byte expected[10];
  size_t size = 0;
  uint64_t remaining = value;
  do {
    uint8_t payload = remaining & 0x7F;
    remaining >>= 7;
    expected[size++] = static_cast<byte>(payload | (remaining != 0 ? 0x80 : 0));
  } while (remaining != 0);
  byte encoded[10];
  MemoryOutputIterator output(encoded, encoded + size);
  WriteVarU64(output, value);
  ASSERT_EQ(kOk, output.status());
  ASSERT_EQ(encoded + size, output.ptr());
  EXPECT_EQ(0, memcmp(expected, encoded, size));
  MemoryIterator input(expected, expected + size);
  uint64_t decoded = 0;
  ASSERT_TRUE(ReadVarU64(input, decoded));
  EXPECT_EQ(value, decoded);
  EXPECT_EQ(expected + size, input.ptr());
  if (value <= UINT32_MAX) {
    MemoryOutputIterator output32(encoded, encoded + size);
    WriteVarU32(output32, static_cast<uint32_t>(value));
    EXPECT_EQ(kOk, output32.status());
    EXPECT_EQ(encoded + size, output32.ptr());
    EXPECT_EQ(0, memcmp(expected, encoded, size));
    MemoryIterator input32(expected, expected + size);
    uint32_t decoded32 = 0;
    ASSERT_TRUE(ReadVarU32(input32, decoded32));
    EXPECT_EQ(value, decoded32);
  }
}

}  // namespace

// Verifies every bit boundary, especially byte five's split across words.
TEST(Write, VarintWordBoundaries) {
  CheckVarint(0);
  CheckVarint(UINT64_MAX);
  for (unsigned bit = 0; bit < 64; ++bit) {
    uint64_t value = uint64_t{1} << bit;
    CheckVarint(value - 1);
    CheckVarint(value);
    CheckVarint(value + 1);
  }
}

// Verifies mixed low/high words against the reference with deterministic data.
TEST(Write, VarintReferenceCorpus) {
  uint64_t state = 0x123456789ABCDEF0ULL;
  for (unsigned i = 0; i < 20000; ++i) {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    CheckVarint(state);
    CheckVarint(static_cast<uint32_t>(state));
  }
}

// Verifies every varint length survives one-byte bulk transfers.
TEST(Write, VarintShortBulkWrites) {
  for (unsigned length = 1; length <= 10; ++length) {
    uint64_t value =
        length == 10 ? UINT64_MAX : (uint64_t{1} << (7 * length)) - 1;
    byte expected[10] = {};
    MemoryOutputIterator reference(expected, expected + 10);
    WriteVarU64(reference, value);
    ShortVarintSink sink;
    WriteVarU64(sink, value);
    EXPECT_EQ(kOk, sink.status());
    ASSERT_EQ(length, sink.size);
    EXPECT_EQ(0, memcmp(expected, sink.data, length));
  }
}

// Verifies errors stop retries and preserve the accepted varint prefix.
TEST(Write, VarintShortBulkFailure) {
  ShortVarintSink sink;
  sink.limit = 3;
  WriteVarU64(sink, UINT64_MAX);
  EXPECT_EQ(kNoSpaceLeftOnDevice, sink.status());
  EXPECT_EQ(3U, sink.size);
  EXPECT_EQ(byte{0xff}, sink.data[2]);
  WriteVarU64(sink, 150);
  EXPECT_EQ(3U, sink.size);
}

struct DrippingIterator {
  byte* data;
  byte* end;
  size_t write(const byte* buf, size_t count) {
    if (count > 3) count = 3;
    const size_t available = static_cast<size_t>(end - data);
    if (count > available) count = available;
    memcpy(data, buf, count);
    data += count;
    return count;
  }
};

TEST(Write, ByteArray) {
  uint8_t result[] = {0, 0, 0, 0, 0, 0, 0, 0};
  DrippingIterator itr{(byte*)result, (byte*)result + 8};
  uint8_t in[] = {1, 2, 3, 4, 5, 6, 7, 8};
  EXPECT_EQ(5, WriteByteArray(itr, (const byte*)in, 5));
  EXPECT_THAT(result, ElementsAre(1, 2, 3, 4, 5, 0, 0, 0));
}

TEST(Write, ByteArrayOverflow) {
  uint8_t result[] = {0, 0, 0, 0, 0, 0, 0, 0};
  DrippingIterator itr{(byte*)result, (byte*)result + 4};
  uint8_t in[] = {1, 2, 3, 4, 5, 6, 7, 8};
  EXPECT_EQ(4, WriteByteArray(itr, (const byte*)in, 10));
  EXPECT_THAT(result, ElementsAre(1, 2, 3, 4, 0, 0, 0, 0));
}

TEST(Write, NativeOverflow) {
  uint8_t result[] = {0, 0, 0, 0, 0, 0, 0, 0};
  MemoryOutputIterator itr{(byte*)result, (byte*)result + 2};
  float f = 3.14159265;
  HostNativeWriter<float>().write(itr, f);
  EXPECT_EQ(0, result[3]);
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
}

TEST(Write, EmptyString) {
  uint8_t result[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr{(byte*)result, (byte*)result + 8};
  WriteString(itr, std::string());
  ASSERT_EQ(kOk, itr.status());
  EXPECT_THAT(result, ElementsAre(0, 9, 9, 9, 9, 9, 9, 9));
}

TEST(Write, SimpleString) {
  uint8_t result[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr{(byte*)result, (byte*)result + 8};
  WriteString(itr, std::string("foo"));
  ASSERT_EQ(kOk, itr.status());
  EXPECT_THAT(result, ElementsAre(3, 'f', 'o', 'o', 9, 9, 9, 9));
}

TEST(Write, CString) {
  uint8_t result[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr{(byte*)result, (byte*)result + 8};
  WriteString(itr, "foo");
  ASSERT_EQ(kOk, itr.status());
  EXPECT_THAT(result, ElementsAre(3, 'f', 'o', 'o', 9, 9, 9, 9));
}

TEST(Write, ArduinoString) {
  uint8_t result[] = {9, 9, 9, 9, 9, 9, 9, 9};
  MemoryOutputIterator itr{(byte*)result, (byte*)result + 8};
  WriteString(itr, String("foo"));
  ASSERT_EQ(kOk, itr.status());
  EXPECT_THAT(result, ElementsAre(3, 'f', 'o', 'o', 9, 9, 9, 9));
}

}  // namespace roo_io
