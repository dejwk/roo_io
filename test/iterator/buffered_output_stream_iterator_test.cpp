#include "roo_io/iterator/buffered_output_stream_iterator.h"

#include <memory>

#include "gtest/gtest.h"
#include "output_iterator_p.h"
#include "roo_io/stream/memory_output_stream.h"

namespace roo_io {

class BufferedOutputStreamIteratorFixture {
 public:
  BufferedOutputStreamIterator createIterator(size_t max_size) {
    contents_ = std::unique_ptr<byte[]>(new byte[max_size]);
    os_ = std::unique_ptr<MemoryOutputStream<byte*>>(
        new MemoryOutputStream<byte*>(contents_.get(),
                                      contents_.get() + max_size));
    return BufferedOutputStreamIterator(*os_);
  }

  std::vector<byte> getResult() const {
    return std::vector<byte>(contents_.get(), os_->ptr());
  }

  std::string getResultAsString() const {
    return std::string((const char*)contents_.get(),
                       (const char*)os_->ptr());
  }

  static constexpr bool strict = true;

 private:
  std::unique_ptr<byte[]> contents_;
  std::unique_ptr<MemoryOutputStream<byte*>> os_;
};

INSTANTIATE_TYPED_TEST_SUITE_P(BufferedOutputStreamIterator, OutputIteratorTest,
                               BufferedOutputStreamIteratorFixture);

TEST(BufferedOutputStreamIterator, DefaultConstructible) {
  BufferedOutputStreamIterator itr;
  EXPECT_EQ(kClosed, itr.status());
  itr.write(byte{5});
  byte buf[] = {byte{1}, byte{2}};
  EXPECT_EQ(0, itr.write(buf, 2));
  EXPECT_EQ(kClosed, itr.status());
}

}  // namespace roo_io