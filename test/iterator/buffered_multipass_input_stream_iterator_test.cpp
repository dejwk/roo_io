#include "roo_io/iterator/buffered_multipass_input_stream_iterator.h"

#include <memory>

#include "gtest/gtest.h"
#include "multipass_input_iterator_p.h"
#include "input_iterator_p.h"
#include "roo_io/stream/memory_input_stream.h"

namespace roo_io {

class BufferedMultipassInputStreamIteratorFixture {
 public:
  BufferedMultipassInputStreamIterator createIterator(const byte* beg,
                                                      size_t size) {
    is_ = std::unique_ptr<MultipassInputStream>(
        new MemoryInputStream<const byte*>(beg, beg + size));
    return BufferedMultipassInputStreamIterator(*is_);
  }

 private:
  std::unique_ptr<MultipassInputStream> is_;
};

INSTANTIATE_TYPED_TEST_SUITE_P(BufferedMultipassInputStreamIterator,
                               InputIteratorTest,
                               BufferedMultipassInputStreamIteratorFixture);

INSTANTIATE_TYPED_TEST_SUITE_P(BufferedMultipassInputStreamIterator,
                               MultipassInputIteratorTest,
                               BufferedMultipassInputStreamIteratorFixture);

}  // namespace roo_io