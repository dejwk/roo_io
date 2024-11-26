#include "roo_io/iterator/memory_input_iterator.h"

#include "gtest/gtest.h"

namespace roo_io {

const uint8_t data[] = "ABCDEFGH";

TEST(UnsafeMemoryIterator, Initialization) {
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(itr.ptr(), data);
}

TEST(UnsafeMemoryIterator, ReadByByte) {
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('B', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('C', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('D', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('E', itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TEST(UnsafeMemoryIterator, ReadArray) {
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ('A', itr.read());
  uint8_t buf[] = "BCD";
  EXPECT_EQ(3, itr.read(buf, 3));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('B', buf[0]);
  EXPECT_EQ('C', buf[1]);
  EXPECT_EQ('D', buf[2]);
  EXPECT_EQ('E', itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TEST(UnsafeMemoryIterator, Skip) {
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ('A', itr.read());
  itr.skip(5);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('G', itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TEST(SafeMemoryIterator, Initialization) {
  MemoryIterator itr(data, data + 8);
  EXPECT_EQ(itr.ptr(), data);
}

TEST(SafeMemoryIterator, Empty) {
  MemoryIterator itr(data, data);
  EXPECT_EQ(kOk, itr.status());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
}

TEST(SafeMemoryIterator, ReadByByte) {
  MemoryIterator itr(data, data + 3);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('B', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('C', itr.read());
  EXPECT_EQ(kOk, itr.status());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
}

TEST(SafeMemoryIterator, ReadArray) {
  MemoryIterator itr(data, data + 8);
  EXPECT_EQ('A', itr.read());
  uint8_t buf[] = "BCD";
  EXPECT_EQ(3, itr.read(buf, 3));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('B', buf[0]);
  EXPECT_EQ('C', buf[1]);
  EXPECT_EQ('D', buf[2]);
  EXPECT_EQ('E', itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TEST(SafeMemoryIterator, ReadArrayPastEos) {
  MemoryIterator itr(data, data + 3);
  EXPECT_EQ('A', itr.read());
  uint8_t buf[] = "BCD";
  EXPECT_EQ(2, itr.read(buf, 3));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('B', buf[0]);
  EXPECT_EQ('C', buf[1]);
  EXPECT_EQ(0, itr.read(buf, 3));
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(0, itr.read(buf, 3));
  EXPECT_EQ(kEndOfStream, itr.status());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
}

TEST(SafeMemoryIterator, Skip) {
  MemoryIterator itr(data, data + 8);
  EXPECT_EQ('A', itr.read());
  itr.skip(5);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ('G', itr.read());
  EXPECT_EQ(kOk, itr.status());
}

TEST(SafeMemoryIterator, SkipPastEos) {
  MemoryIterator itr(data, data + 8);
  EXPECT_EQ('A', itr.read());
  itr.skip(5);
  EXPECT_EQ(kOk, itr.status());
  itr.skip(5);
  EXPECT_EQ(kEndOfStream, itr.status());
  itr.skip(3);
  EXPECT_EQ(kEndOfStream, itr.status());
}

TEST(MultipassMemoryIterator, Initialization) {
  MultipassMemoryIterator itr(data, data + 8);
  EXPECT_EQ(data, itr.ptr());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(8, itr.size());
}

TEST(MultipassMemoryIterator, Empty) {
  MultipassMemoryIterator itr(data, data);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(0, itr.size());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(0, itr.size());
}

TEST(MultipassMemoryIterator, ReadByByteWithRewind) {
  MultipassMemoryIterator itr(data, data + 3);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('B', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.rewind();
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('B', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('C', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.rewind();
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
}

TEST(MultipassMemoryIterator, ReadByByteWithSeek) {
  MultipassMemoryIterator itr(data, data + 3);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.seek(2);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('C', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.seek(1);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('B', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.seek(3);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.read();
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.seek(1);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('B', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.seek(4);
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.seek(100);
  EXPECT_EQ(kEndOfStream, itr.status());
  EXPECT_EQ(3, itr.position());
  EXPECT_EQ(3, itr.size());
  itr.seek(0);
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(0, itr.position());
  EXPECT_EQ(3, itr.size());
  EXPECT_EQ('A', itr.read());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(1, itr.position());
  EXPECT_EQ(3, itr.size());
}

}