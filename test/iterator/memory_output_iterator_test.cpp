#include "roo_io/iterator/memory_output_iterator.h"

#include "gtest/gtest.h"
#include "output_iterator_p.h"

namespace roo_io {

// Keeps track of the count of bytes written.
struct TestIterator {
  TestIterator(uint64_t& total_count, byte* out, byte* end)
      : itr(out, end), total_count(total_count) {}

  MemoryOutputIterator itr;
  uint64_t& total_count;

  void write(byte v) {
    itr.write(v);
    if (itr.status() == kOk) ++total_count;
  }

  size_t write(const byte* buf, size_t count) {
    size_t written = itr.write(buf, count);
    total_count += written;
    return written;
  }

  Status status() const { return itr.status(); }

  void flush() { itr.flush(); }
};

class MemoryOutputIteratorFixture {
 public:
  TestIterator createIterator(size_t max_size) {
    max_size_ = max_size;
    contents_ = std::unique_ptr<byte[]>(new byte[max_size + 16]);
    memset(contents_.get(), 0, max_size + 16);
    written_ = 0;
    return TestIterator(written_, contents_.get(), contents_.get() + max_size);
  }

  std::string getResultAsString() const {
    return std::string((const char*)contents_.get(), written_);
  }

  std::vector<byte> getResult() const {
    return std::vector<byte>(&contents_[0], &contents_[written_]);
  }

  static constexpr bool strict = true;

 private:
  size_t max_size_;
  std::unique_ptr<byte[]> contents_;
  std::unique_ptr<MemoryOutputIterator> itr_;
  uint64_t written_;
};

INSTANTIATE_TYPED_TEST_SUITE_P(MemoryOutputIterator, OutputIteratorTest,
                               MemoryOutputIteratorFixture);

}  // namespace roo_io