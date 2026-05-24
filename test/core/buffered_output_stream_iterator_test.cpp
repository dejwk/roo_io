#include "roo_io/core/buffered_output_stream_iterator.h"

#include <memory>

#include "gtest/gtest.h"
#include "output_iterator_p.h"
#include "roo_io/memory/memory_output_stream.h"

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
    return std::string((const char*)contents_.get(), (const char*)os_->ptr());
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

TEST(BufferedOutputStreamIterator, BlockWriteAfterFlushFailureReturnsZero) {
  byte data[63] = {};
  MemoryOutputStream<byte*> os(data, data + 63);
  BufferedOutputStreamIterator itr(os);

  std::string contents(63, 'A');
  EXPECT_EQ(63, itr.write(reinterpret_cast<const byte*>(contents.data()),
                          contents.size()));
  EXPECT_EQ(1, itr.write(reinterpret_cast<const byte*>("B"), 1));
  EXPECT_EQ(kOk, itr.status());

  EXPECT_EQ(0, itr.write(reinterpret_cast<const byte*>("CD"), 2));
  EXPECT_EQ(kNoSpaceLeftOnDevice, itr.status());
  EXPECT_EQ(std::string(63, 'A'),
            std::string(reinterpret_cast<const char*>(data),
                        reinterpret_cast<const char*>(os.ptr())));
}

}  // namespace roo_io