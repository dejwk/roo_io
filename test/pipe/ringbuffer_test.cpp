#include "roo_io/pipe/ringbuffer.h"

#include "gtest/gtest.h"

namespace roo_io {

TEST(RingBufferTest, EmptyBuffer) {
  RingBuffer buf(4);
  EXPECT_TRUE(buf.empty());
  EXPECT_EQ(buf.used(), 0);
  EXPECT_TRUE(buf.empty());
  EXPECT_EQ(buf.capacity(), 4);
  EXPECT_EQ(buf.free(), 4);
}

TEST(RingBufferTest, Write) {
  RingBuffer buf(4);
  byte data[] = {byte{1}, byte{2}, byte{3}};
  size_t written = buf.write(data, 3);
  EXPECT_EQ(written, 3);
  EXPECT_EQ(buf.used(), 3);
  EXPECT_EQ(buf.free(), 1);
}

TEST(RingBufferTest, BufferOverflow) {
  RingBuffer buf(2);
  byte data[] = {byte{1}, byte{2}, byte{3}};
  size_t written = buf.write(data, 3);
  EXPECT_EQ(written, 2);
  EXPECT_EQ(buf.used(), 2);
  EXPECT_EQ(buf.free(), 0);
  written = buf.write(data, 3);
  EXPECT_EQ(written, 0);  // No space left.
}

TEST(RingBufferTest, BufferUnderflow) {
  RingBuffer buf(2);
  EXPECT_TRUE(buf.empty());
  byte out[2];
  size_t read = buf.read(out, 2);
  EXPECT_EQ(read, 0);  // Nothing to read.
}

TEST(RingBufferTest, WrapAround) {
  RingBuffer buf(3);
  byte data1[] = {byte{1}, byte{2}};
  size_t written = buf.write(data1, 2);
  EXPECT_EQ(written, 2);
  EXPECT_EQ(buf.used(), 2);
  EXPECT_EQ(buf.free(), 1);
  byte out[2];
  size_t read = buf.read(out, 1);
  EXPECT_EQ(read, 1);
  EXPECT_EQ(out[0], byte{1});
  EXPECT_EQ(buf.used(), 1);
  EXPECT_EQ(buf.free(), 2);
  byte data2[] = {byte{3}, byte{4}};
  written = buf.write(data2, 2);
  EXPECT_EQ(written, 2);
  EXPECT_EQ(buf.used(), 3);
  EXPECT_EQ(buf.free(), 0);
  byte out2[3];
  read = buf.read(out2, 3);
  EXPECT_EQ(read, 3);
  EXPECT_EQ(out2[0], byte{2});
  EXPECT_EQ(out2[1], byte{3});
  EXPECT_EQ(out2[2], byte{4});
  EXPECT_TRUE(buf.empty());
  EXPECT_EQ(buf.free(), 3);
}

}  // namespace roo_io