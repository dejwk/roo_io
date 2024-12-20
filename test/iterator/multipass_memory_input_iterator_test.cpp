#include "gtest/gtest.h"
#include "input_iterator_p.h"
#include "multipass_input_iterator_p.h"
#include "roo_io/memory/memory_input_iterator.h"

namespace roo_io {

class MultipassMemoryInputIteratorFixture {
 public:
  using Iterator = MultipassMemoryIterator;

  MultipassMemoryIterator createIterator(const byte* beg, size_t size) {
    return MultipassMemoryIterator(beg, beg + size);
  }
};

INSTANTIATE_TYPED_TEST_SUITE_P(MultipassMemoryInputIterator, InputIteratorTest,
                               MultipassMemoryInputIteratorFixture);

INSTANTIATE_TYPED_TEST_SUITE_P(MultipassMemoryInputIterator,
                               MultipassInputIteratorTest,
                               MultipassMemoryInputIteratorFixture);

}  // namespace roo_io