
#include "roo_io/memory/fill.h"

#include <memory>

#include "gtest/gtest.h"

using namespace testing;

// std::default_random_engine generator;

namespace roo_io {

void TrivialPatternFill(byte* buf, const byte* val, int pattern_size,
                        uint32_t repetitions) {
  while (repetitions-- > 0) {
    for (int i = 0; i < pattern_size; ++i) {
      *buf++ = val[i];
    }
  }
}

namespace {

// Sets the nth bit (as specified by offset) in the specified buffer,
// to the specified value.
inline void fillBit(byte* buf, uint32_t offset, bool value) {
  buf += (offset / 8);
  offset %= 8;
  if (value) {
    *buf |= (byte)(1 << offset);
  } else {
    *buf &= (byte)(~(1 << offset));
  }
}

// Sets the nth nibble (as specified by offset) in the specified buffer,
// to the specified value.
inline void fillNibble(byte* buf, uint32_t offset, byte value) {
  buf += (offset / 2);
  offset %= 2;
  if (offset == 0) {
    *buf &= byte{0x0F};
    *buf |= (value << 4);
  } else {
    *buf &= byte{0xF0};
    *buf |= value;
  }
}

}  // namespace

void TrivialBitFill(byte* buf, uint32_t offset, int16_t count, bool value) {
  while (count-- > 0) {
    fillBit(buf, offset++, value);
  }
}

void TrivialNibbleFill(byte* buf, uint32_t offset, int16_t count, byte value) {
  while (count-- > 0) {
    fillNibble(buf, offset++, value);
  }
}

class ByteBuffer {
 public:
  ByteBuffer(uint32_t size) : size_(size), data_(new byte[size + 1]) {
    memset(data_.get(), 0, size + 1);
  }

  uint32_t size() const { return size_; }
  const byte* data() const { return data_.get(); }
  byte* data() { return data_.get(); }

  ~ByteBuffer() {
    EXPECT_EQ(byte{0}, data_.get()[size_]) << "Write-over detected";
  }

 private:
  uint32_t size_;
  std::unique_ptr<byte[]> data_;
};

std::ostream& operator<<(std::ostream& os, const ByteBuffer& buf) {
  for (uint32_t i = 0; i < buf.size(); ++i) {
    if (i > 0) os << ", ";
    os << (int)buf.data()[i];
  }
  return os;
}

bool operator==(const ByteBuffer& a, const ByteBuffer& b) {
  for (uint32_t i = 0; i < a.size(); ++i) {
    if (a.data()[i] != b.data()[i]) return false;
  }
  return true;
}

class FillerTester {
 public:
  FillerTester(uint32_t size) : size_(size), actual_(size), expected_(size) {}

  void patternFill2(uint32_t pos, uint32_t count, const byte* val) {
    TrivialPatternFill(expected_.data() + pos, val, 2, count);
    PatternFill<2>(actual_.data() + pos, count, val);
    EXPECT_EQ(expected_, actual_);
  }

  void patternFill3(uint32_t pos, uint32_t count, const byte* val) {
    TrivialPatternFill(expected_.data() + pos, val, 3, count);
    PatternFill<3>(actual_.data() + pos, count, val);
    EXPECT_EQ(expected_, actual_);
  }

  void patternFill4(uint32_t pos, uint32_t count, const byte* val) {
    TrivialPatternFill(expected_.data() + pos, val, 4, count);
    PatternFill<4>(actual_.data() + pos, count, val);
    EXPECT_EQ(expected_, actual_);
  }

  void bitFill(uint32_t offset, int16_t count, bool value) {
    TrivialBitFill(expected_.data(), offset, count, value);
    BitFill(actual_.data(), offset, count, value);
    EXPECT_EQ(expected_, actual_);
  }

  void nibbleFill(uint32_t offset, int16_t count, byte value) {
    EXPECT_LT((int)value, 16);
    TrivialNibbleFill(expected_.data(), offset, count, value);
    NibbleFill(actual_.data(), offset, count, value);
    EXPECT_EQ(expected_, actual_);
  }

 private:
  uint32_t size_;
  ByteBuffer actual_;
  ByteBuffer expected_;
};

byte pattern[] = {byte{0x12}, byte{0x34}, byte{0x56}, byte{0x78},
                  byte{0x9A}, byte{0xBC}, byte{0xDE}, byte{0xF0}};

TEST(PatternFill2, Empty) {
  FillerTester tester(32);
  tester.patternFill2(4, 0, pattern);
  tester.patternFill2(5, 0, pattern + 2);
}

TEST(PatternFill2, Seven) {
  FillerTester tester(32);
  tester.patternFill2(5, 7, pattern);
}

TEST(PatternFill2, Eight) {
  FillerTester tester(32);
  tester.patternFill2(5, 8, pattern);
}

TEST(PatternFill2, ShortAligned) {
  FillerTester tester(32);
  tester.patternFill2(4, 1, pattern);
  tester.patternFill2(8, 3, pattern + 2);
}

TEST(PatternFill2, ShortMisaligned) {
  FillerTester tester(32);
  tester.patternFill2(3, 1, pattern);
  tester.patternFill2(5, 3, pattern + 2);
}

TEST(PatternFill2, LongAligned) {
  FillerTester tester(32);
  tester.patternFill2(4, 12, pattern);
}

TEST(PatternFill2, LongMisaligned) {
  FillerTester tester(32);
  tester.patternFill2(3, 12, pattern);
}

TEST(PatternFill2, LongMisalignedEq) {
  FillerTester tester(32);
  byte pattern[] = {byte{0x12}, byte{0x12}};
  tester.patternFill2(3, 12, pattern);
}

// Fill3

TEST(PatternFill3, Empty) {
  FillerTester tester(32);
  tester.patternFill3(4, 0, pattern);
  tester.patternFill3(5, 0, pattern + 2);
}

TEST(PatternFill3, Seven) {
  FillerTester tester(32);
  tester.patternFill3(5, 7, pattern);
}

TEST(PatternFill3, Eight) {
  FillerTester tester(32);
  tester.patternFill3(5, 8, pattern);
}

TEST(PatternFill3, ShortAligned) {
  FillerTester tester(32);
  tester.patternFill3(4, 1, pattern);
  tester.patternFill3(8, 3, pattern + 2);
}

TEST(PatternFill3, ShortMisaligned) {
  FillerTester tester(32);
  tester.patternFill3(3, 1, pattern);
  tester.patternFill3(5, 3, pattern + 2);
}

TEST(PatternFill3, LongAligned) {
  FillerTester tester(50);
  tester.patternFill3(4, 12, pattern);
}

TEST(PatternFill3, LongMisaligned) {
  FillerTester tester(50);
  tester.patternFill3(3, 12, pattern);
}

TEST(PatternFill3, LongMisalignedEq) {
  FillerTester tester(50);
  byte pattern[] = {byte{0x12}, byte{0x12}};
  tester.patternFill3(3, 12, pattern);
}

TEST(PatternFill3, VeryLongAligned) {
  FillerTester tester(80);
  tester.patternFill3(4, 19, pattern);
}

TEST(PatternFill3, VeryLongMisaligned) {
  FillerTester tester(80);
  tester.patternFill3(3, 19, pattern);
}

TEST(PatternFill3, VeryLongMisalignedEq) {
  FillerTester tester(80);
  byte pattern[] = {byte{0x12}, byte{0x12}};
  tester.patternFill3(3, 19, pattern);
}

// Fill4

TEST(PatternFill4, Empty) {
  FillerTester tester(32);
  tester.patternFill4(4, 0, pattern);
  tester.patternFill4(5, 0, pattern + 2);
}

TEST(PatternFill4, Seven) {
  FillerTester tester(50);
  tester.patternFill4(5, 7, pattern);
}

TEST(PatternFill4, Eight) {
  FillerTester tester(50);
  tester.patternFill4(5, 8, pattern);
}

TEST(PatternFill4, ShortAligned) {
  FillerTester tester(50);
  tester.patternFill4(4, 1, pattern);
  tester.patternFill4(8, 3, pattern + 2);
}

TEST(PatternFill4, ShortMisaligned) {
  FillerTester tester(50);
  tester.patternFill4(3, 1, pattern);
  tester.patternFill4(5, 3, pattern + 2);
}

TEST(PatternFill4, LongAligned) {
  FillerTester tester(60);
  tester.patternFill4(4, 12, pattern);
}

TEST(PatternFill4, LongMisaligned1) {
  FillerTester tester(60);
  tester.patternFill4(5, 11, pattern);
}

TEST(PatternFill4, LongMisaligned2) {
  FillerTester tester(60);
  tester.patternFill4(6, 11, pattern);
}

TEST(PatternFill4, LongMisaligned3) {
  FillerTester tester(60);
  tester.patternFill4(7, 11, pattern);
}

TEST(PatternFill4, LongMisalignedEq) {
  FillerTester tester(60);
  byte pattern[] = {byte{0x12}, byte{0x12}};
  tester.patternFill4(3, 12, pattern);
}

TEST(PatternFill4, VeryLongAligned) {
  FillerTester tester(100);
  tester.patternFill4(4, 19, pattern);
}

TEST(PatternFill4, VeryLongMisaligned) {
  FillerTester tester(100);
  tester.patternFill4(3, 19, pattern);
}

TEST(PatternFill4, VeryLongMisalignedEq) {
  FillerTester tester(100);
  byte pattern[] = {byte{0x12}, byte{0x12}};
  tester.patternFill4(5, 19, pattern);
}

TEST(BitFill, Simple) {
  FillerTester tester(100);
  tester.bitFill(5, 19, true);
  tester.bitFill(25, 56, true);
  tester.bitFill(10, 40, false);
  tester.bitFill(120, 120, true);
  tester.bitFill(121, 3, false);
  tester.bitFill(122, 4, true);
}

TEST(NibbleFill, Simple) {
  FillerTester tester(100);
  tester.nibbleFill(5, 19, byte{1});
  tester.nibbleFill(25, 7, byte{5});
  tester.nibbleFill(10, 40, byte{14});
  tester.nibbleFill(12, 12, byte{8});
  tester.nibbleFill(8, 21, byte{11});
  tester.nibbleFill(43, 1, byte{15});
  tester.nibbleFill(33, 3, byte{12});
}

}  // namespace roo_io
