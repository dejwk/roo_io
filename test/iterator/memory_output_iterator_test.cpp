#include "roo_io/iterator/memory_output_iterator.h"

#include "gtest/gtest.h"

namespace roo_io {

TEST(MemoryOutputIterator, Initialization) {
  byte buf[5];
  MemoryOutputIterator itr(buf, buf + 5);
  EXPECT_EQ(buf, itr.ptr());
  EXPECT_EQ(kOk, itr.status());
}

TEST(MemoryOutputIterator, Empty) {
  byte buf[] = "ABCDE";
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
  byte buf[] = "        ";
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
  byte buf[] = "        ";
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
  byte buf[] = "        ";
  MemoryOutputIterator itr(buf, buf + 5);
  EXPECT_EQ(buf, itr.ptr());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(3, itr.write((const byte*)"ABC", 3));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.write((const byte*)"DE", 2));
  EXPECT_EQ(kOk, itr.status());
  itr.write('D');
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  itr.write('E');
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  EXPECT_STREQ("ABCDE   ", (const char*)buf);
}

TEST(MemoryOutputIterator, WriteByBlockPastCapacity) {
  byte buf[] = "        ";
  MemoryOutputIterator itr(buf, buf + 5);
  EXPECT_EQ(buf, itr.ptr());
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(3, itr.write((const byte*)"ABC", 3));
  EXPECT_EQ(kOk, itr.status());
  EXPECT_EQ(2, itr.write((const byte*)"DEF", 3));
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  EXPECT_EQ(0, itr.write((const byte*)"GHI", 3));
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  EXPECT_STREQ("ABCDE   ", (const char*)buf);
}

TEST(MemoryOutputIterator, Movable) {
  byte buf[] = "        ";
  MemoryOutputIterator itr(buf, buf + 8);
  EXPECT_EQ(buf, itr.ptr());
  EXPECT_EQ(kOk, itr.status());
  itr.write('A');
  EXPECT_EQ(kOk, itr.status());
  MemoryOutputIterator itr2 = std::move(itr);
  EXPECT_EQ(kOk, itr2.status());
  itr2.write('B');
  EXPECT_EQ(kOk, itr2.status());
  itr2.write('C');
  EXPECT_EQ(kOk, itr2.status());
  EXPECT_STREQ("ABC     ", (const char*)buf);
}

}