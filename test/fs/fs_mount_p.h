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
    EXPECT_TRUE(fs.isInUse());
  }
  EXPECT_FALSE(fs.isMounted());
  EXPECT_FALSE(fs.isInUse());
}

TYPED_TEST_P(FsMountTest, LazyMountLazyUnmount) {
  Filesystem& fs = this->fs();
  fs.setUnmountingPolicy(Filesystem::kLazyUnmount);
  EXPECT_FALSE(fs.isMounted());
  {
    Mount m = fs.mount();
    ASSERT_TRUE(m.ok());
    EXPECT_TRUE(fs.isMounted());
    EXPECT_TRUE(fs.isInUse());
  }
  EXPECT_FALSE(fs.isInUse());
  EXPECT_TRUE(fs.isMounted());
  fs.setUnmountingPolicy(Filesystem::kEagerUnmount);
  EXPECT_FALSE(fs.isInUse());
  EXPECT_FALSE(fs.isMounted());
}

TYPED_TEST_P(FsMountTest, UnmountPolicySwitch) {
  Filesystem& fs = this->fs();
  EXPECT_FALSE(fs.isMounted());
  {
    Mount m = fs.mount();
    ASSERT_TRUE(m.ok());
    EXPECT_TRUE(fs.isMounted());
    EXPECT_TRUE(fs.isInUse());
    fs.setUnmountingPolicy(Filesystem::kLazyUnmount);
  }
  EXPECT_FALSE(fs.isInUse());
  EXPECT_TRUE(fs.isMounted());
  fs.setUnmountingPolicy(Filesystem::kEagerUnmount);
  EXPECT_FALSE(fs.isInUse());
  EXPECT_FALSE(fs.isMounted());
}

REGISTER_TYPED_TEST_SUITE_P(FsMountTest, LazyMountEagerUnmount,
                            LazyMountLazyUnmount, UnmountPolicySwitch);

}  // namespace roo_io
