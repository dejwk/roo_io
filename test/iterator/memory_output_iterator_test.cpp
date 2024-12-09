#include "roo_io/iterator/memory_output_iterator.h"

#include "gtest/gtest.h"

#include "output_iterator_p.h"

namespace roo_io {

class MemoryOutputIteratorFixture {
 public:
  MemoryOutputIterator createIterator(byte* beg, size_t size) {
    return MemoryOutputIterator(beg, beg + size);
  }
};

INSTANTIATE_TYPED_TEST_SUITE_P(MemoryOutputIterator, OutputIteratorTest,
                               MemoryOutputIteratorFixture);

}