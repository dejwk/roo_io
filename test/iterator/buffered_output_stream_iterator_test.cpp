#include "roo_io/iterator/buffered_output_stream_iterator.h"

#include <memory>

#include "gtest/gtest.h"
#include "output_iterator_p.h"
#include "roo_io/stream/memory_output_stream.h"

namespace roo_io {

class BufferedOutputStreamIteratorFixture {
 public:
  BufferedOutputStreamIterator createIterator(size_t max_size) {
    contents_ = std::unique_ptr<char[]>(new char[max_size]);
    os_ = std::unique_ptr<MemoryOutputStream<byte*>>(
        new MemoryOutputStream<byte*>((byte*)contents_.get(),
                                      (byte*)contents_.get() + max_size));
    return BufferedOutputStreamIterator(*os_);
  }

  std::string getResult() const {
    return std::string(contents_.get(), (char*)os_->ptr() - contents_.get());
  }

 private:
  std::unique_ptr<char[]> contents_;
  std::unique_ptr<MemoryOutputStream<byte*>> os_;
};

INSTANTIATE_TYPED_TEST_SUITE_P(BufferedOutputStreamIterator, OutputIteratorTest,
                               BufferedOutputStreamIteratorFixture);

}  // namespace roo_io