#include "gtest/gtest.h"
#include "roo_io/data/reader.h"
#include "roo_io/iterator/memory_input_iterator.h"

namespace roo_io {

TEST(DramStream, Reads) {
  uint8_t data[] = {0x23, 0xF5};
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(0x23, itr.read());
  EXPECT_EQ(0xF5, itr.read());
}

TEST(DramStream, SkipsAndResets) {
  uint8_t data[] = {0x23, 0xF5, 0xE3, 0x43};
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(0x23, itr.read());
  itr.skip(2);
  EXPECT_EQ(0x43, itr.read());
  // stream.seek(1);
  // EXPECT_EQ(0xF5, stream.read());
}

TEST(DramStream, ReadsUint16) {
  uint8_t data[] = {0x23, 0xF5};
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(0x23F5, ReadU16be(itr));
}

TEST(DramStream, ReadsUint32) {
  uint8_t data[] = {0x23, 0xF5, 0xE3, 0x43};
  UnsafeMemoryIterator itr(data);
  EXPECT_EQ(0x23F5E343, ReadU32be(itr));
}

}  // namespace roo_io