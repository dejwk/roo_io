#include "roo_io/iterator/memory_output_iterator.h"

#include "gtest/gtest.h"
#include "output_iterator_p.h"

namespace roo_io {

class MemoryOutputIteratorFixture {
 public:
  MemoryOutputIterator createIterator(size_t max_size) {
    contents_ = std::unique_ptr<char[]>(new char[max_size + 16]);
    memset(contents_.get(), 0, max_size + 16);
    itr_ = std::unique_ptr<MemoryOutputIterator>(
        new MemoryOutputIterator((byte*)contents_.get(), (byte*)contents_.get() + max_size));
    return *itr_;
  }

  std::string getResult() const {
    return (const char*)contents_.get();
  }

  static constexpr bool strict = true;

 private:
  std::unique_ptr<char[]> contents_;
  std::unique_ptr<MemoryOutputIterator> itr_;
};

INSTANTIATE_TYPED_TEST_SUITE_P(MemoryOutputIterator, OutputIteratorTest,
                               MemoryOutputIteratorFixture);

}  // namespace roo_io