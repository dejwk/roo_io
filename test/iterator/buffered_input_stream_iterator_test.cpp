#include "roo_io/core/buffered_input_stream_iterator.h"
#include <memory>

#include "gtest/gtest.h"
#include "input_iterator_p.h"
#include "roo_io/memory/memory_input_stream.h"

namespace roo_io {

class BufferedInputStreamIteratorFixture {
 public:
  using Iterator = BufferedInputStreamIterator;

  BufferedInputStreamIterator createIterator(const byte* beg, size_t size) {
    is_ = std::unique_ptr<InputStream>(
        new MemoryInputStream<const byte*>(beg, beg + size));
    return BufferedInputStreamIterator(*is_);
  }

 private:
  std::unique_ptr<InputStream> is_;
};

INSTANTIATE_TYPED_TEST_SUITE_P(BufferedInputStreamIterator, InputIteratorTest,
                               BufferedInputStreamIteratorFixture);

}  // namespace roo_io