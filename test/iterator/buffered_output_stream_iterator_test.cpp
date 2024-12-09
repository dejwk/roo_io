#include "roo_io/iterator/buffered_output_stream_iterator.h"

#include <memory>

#include "gtest/gtest.h"
#include "output_iterator_p.h"
#include "roo_io/stream/memory_output_stream.h"

namespace roo_io {

class BufferedOutputStreamIteratorFixture {
 public:
  BufferedOutputStreamIterator createIterator(byte* beg, size_t size) {
    is_ = std::unique_ptr<OutputStream>(
        new MemoryOutputStream<byte*>(beg, beg + size));
    return BufferedOutputStreamIterator(*is_);
  }

 private:
  std::unique_ptr<OutputStream> is_;
};

INSTANTIATE_TYPED_TEST_SUITE_P(BufferedOutputStreamIterator, OutputIteratorTest,
                               BufferedOutputStreamIteratorFixture);

}  // namespace roo_io