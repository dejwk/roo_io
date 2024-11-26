#include "roo_io/iterator/memory_output_iterator.h"

#include "gtest/gtest.h"

namespace roo_io {

TEST(MemoryOutputIterator, Initialization) {
  uint8_t buf[5];
  MemoryOutputIterator itr(buf, buf + 5);
  EXPECT_EQ(buf, itr.ptr());
  EXPECT_EQ(kOk, itr.status());
}

TEST(MemoryOutputIterator, Empty) {
  uint8_t buf[] = "ABCDE";
  MemoryOutputIterator itr(buf, buf);
  EXPECT_EQ(buf, itr.ptr());
  EXPECT_EQ(kOk, itr.status());
  itr.write('A');
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  itr.write('B');
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  EXPECT_STREQ("ABCDE", (const char*)buf);
}

TEST(MemoryOutputIterator, WriteByByte) {
  uint8_t buf[] = "        ";
  MemoryOutputIterator itr(buf, buf + 8);
  EXPECT_EQ(buf, itr.ptr());
  EXPECT_EQ(kOk, itr.status());
  itr.write('A');
  EXPECT_EQ(kOk, itr.status());
  itr.write('B');
  EXPECT_EQ(kOk, itr.status());
  itr.write('C');
  EXPECT_EQ(kOk, itr.status());
  EXPECT_STREQ("ABC     ", (const char*)buf);
}

TEST(MemoryOutputIterator, WriteByBytePastCapacity) {
  uint8_t buf[] = "        ";
  MemoryOutputIterator itr(buf, buf + 3);
  EXPECT_EQ(buf, itr.ptr());
  EXPECT_EQ(kOk, itr.status());
  itr.write('A');
  EXPECT_EQ(kOk, itr.status());
  itr.write('B');
  EXPECT_EQ(kOk, itr.status());
  itr.write('C');
  EXPECT_EQ(kOk, itr.status());
  itr.write('D');
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  itr.write('E');
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  EXPECT_STREQ("ABC     ", (const char*)buf);
}

TEST(MemoryOutputIterator, WriteByBlockTillCapacity) {
  uint8_t buf[] = "        ";
  MemoryOutputIterator itr(buf, buf + 5);
  EXPECT_EQ(buf, itr.ptr());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(3, itr.write((const uint8_t*)"ABC", 3));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.write((const uint8_t*)"DE", 2));
  EXPECT_EQ(kOk, itr.status());
  itr.write('D');
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  itr.write('E');
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  EXPECT_STREQ("ABCDE   ", (const char*)buf);
}

TEST(MemoryOutputIterator, WriteByBlockPastCapacity) {
  uint8_t buf[] = "        ";
  MemoryOutputIterator itr(buf, buf + 5);
  EXPECT_EQ(buf, itr.ptr());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(3, itr.write((const uint8_t*)"ABC", 3));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.write((const uint8_t*)"DEF", 3));
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  EXPECT_EQ(0, itr.write((const uint8_t*)"GHI", 3));
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  EXPECT_STREQ("ABCDE   ", (const char*)buf);
}

}