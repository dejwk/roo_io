#include "gtest/gtest.h"
#include "roo_io/iterator/memory_output_iterator.h"

namespace roo_io {

template <typename ItrFactory>
class OutputIteratorTest : public testing::Test {
 public:
  auto createIterator(const byte* beg, size_t size) {
    return factory.createIterator(beg, size);
  }

 private:
  ItrFactory factory;
};

TYPED_TEST_SUITE_P(OutputIteratorTest);

TYPED_TEST_P(OutputIteratorTest, Initialization) {
  byte buf[5];
  MemoryOutputIterator itr(buf, buf + 5);
  //   EXPECT_EQ(buf, itr.ptr());
  EXPECT_EQ(kOk, itr.status());
}

TYPED_TEST_P(OutputIteratorTest, Empty) {
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

TYPED_TEST_P(OutputIteratorTest, WriteByByte) {
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

TYPED_TEST_P(OutputIteratorTest, WriteByBytePastCapacity) {
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

TYPED_TEST_P(OutputIteratorTest, WriteByBlockTillCapacity) {
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

TYPED_TEST_P(OutputIteratorTest, WriteByBlockPastCapacity) {
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

TYPED_TEST_P(OutputIteratorTest, Movable) {
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

REGISTER_TYPED_TEST_SUITE_P(OutputIteratorTest, Initialization, Empty,
                            WriteByByte, WriteByBytePastCapacity,
                            WriteByBlockTillCapacity, WriteByBlockPastCapacity,
                            Movable);

}  // namespace roo_io