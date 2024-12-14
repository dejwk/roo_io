#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace roo_io {

template <typename Fs>
class FsMountTest : public testing::Test {
 public:
  FsMountTest() : fakefs_(), fs_(fakefs_) {}

  Filesystem& fs() { return fs_; }
  fakefs::FakeFs& fake() { return fakefs_; }

  fakefs::FakeFs fakefs_;
  Fs fs_;
};

TYPED_TEST_SUITE_P(FsMountTest);

TYPED_TEST_P(FsMountTest, LazyMountEagerUnmount) {
  Filesystem& fs = this->fs();
  EXPECT_FALSE(fs.isMounted());
  {
    Mount m = fs.mount();
    ASSERT_TRUE(m.ok());
    EXPECT_TRUE(fs.isMounted());
  }
  EXPECT_FALSE(fs.isMounted());
}

REGISTER_TYPED_TEST_SUITE_P(FsMountTest, LazyMountEagerUnmount);

}  // namespace roo_io
