#include "roo_io/iterator/memory_input_iterator.h"

#include "gtest/gtest.h"
#include "input_iterator_p.h"

namespace roo_io {
class SafeMemoryInputIteratorFixture {
 public:
  MemoryIterator createIterator(const byte* beg, size_t size) {
    return MemoryIterator(beg, beg + size);
  }
};

INSTANTIATE_TYPED_TEST_SUITE_P(MemoryInputIterator, InputIteratorTest,
                               SafeMemoryInputIteratorFixture);

TEST(InputIteratorTest, Initialization) {
  const byte data[] = "ABCDEFGH";
  MemoryIterator itr(data, data + 8);
  EXPECT_EQ(itr.ptr(), data);
}

}  // namespace roo_io