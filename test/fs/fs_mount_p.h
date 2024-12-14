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
  fs.setUnmountingPolicy(Filesystem::kUnmountLazily);
  EXPECT_FALSE(fs.isMounted());
  {
    Mount m = fs.mount();
    ASSERT_TRUE(m.ok());
    EXPECT_TRUE(fs.isMounted());
    EXPECT_TRUE(fs.isInUse());
  }
  EXPECT_FALSE(fs.isInUse());
  EXPECT_TRUE(fs.isMounted());
  fs.setUnmountingPolicy(Filesystem::kUnmountEagerly);
  EXPECT_FALSE(fs.isInUse());
  EXPECT_FALSE(fs.isMounted());
}

TYPED_TEST_P(FsMountTest, UnmountPolicySwitched) {
  Filesystem& fs = this->fs();
  EXPECT_FALSE(fs.isMounted());
  {
    Mount m = fs.mount();
    ASSERT_TRUE(m.ok());
    EXPECT_TRUE(fs.isMounted());
    EXPECT_TRUE(fs.isInUse());
    fs.setUnmountingPolicy(Filesystem::kUnmountLazily);
  }
  EXPECT_FALSE(fs.isInUse());
  EXPECT_TRUE(fs.isMounted());
  fs.setUnmountingPolicy(Filesystem::kUnmountEagerly);
  EXPECT_FALSE(fs.isInUse());
  EXPECT_FALSE(fs.isMounted());
}

TYPED_TEST_P(FsMountTest, ForcedUnmount) {
  Filesystem& fs = this->fs();
  Mount m = fs.mount();
  ASSERT_TRUE(m.ok());
  EXPECT_TRUE(fs.isMounted());
  EXPECT_TRUE(fs.isInUse());

  fs.forceUnmount();
  EXPECT_FALSE(fs.isMounted());
  EXPECT_FALSE(fs.isInUse());
  EXPECT_EQ(kNotMounted, m.status());
  EXPECT_EQ(kNotMounted, m.mkdir("/foo"));

  // New mount should be fine.
  Mount m2 = fs.mount();
  ASSERT_TRUE(m2.ok());
  EXPECT_TRUE(fs.isMounted());
  EXPECT_TRUE(fs.isInUse());
  EXPECT_TRUE(m2.exists("/"));

  // But the original mount should still be deactivated.
  EXPECT_EQ(kNotMounted, m.status());
  EXPECT_EQ(kNotMounted, m.mkdir("/foo"));
}

TYPED_TEST_P(FsMountTest, ReadWriteReadOnlyReadWrite) {
  Filesystem& fs = this->fs();
  Mount rw1 = fs.mount();
  ASSERT_TRUE(rw1.ok());
  EXPECT_TRUE(fs.isMounted());
  EXPECT_TRUE(fs.isInUse());
  EXPECT_FALSE(rw1.isReadOnly());
  EXPECT_EQ(kOk, rw1.mkdir("/foo"));

  fs.setMountingPolicy(Filesystem::kMountReadOnly);
  Mount ro1 = fs.mount();
  ASSERT_TRUE(ro1.ok());
  EXPECT_TRUE(ro1.isReadOnly());
  EXPECT_EQ(kReadOnlyFilesystem, ro1.mkdir("/bar"));

  fs.setMountingPolicy(Filesystem::kMountReadWrite);
  Mount rw2 = fs.mount();
  ASSERT_TRUE(rw2.ok());
  EXPECT_FALSE(rw2.isReadOnly());
  EXPECT_EQ(kOk, rw2.mkdir("/baz"));

  // Re-check that the ro is still read-only.
  EXPECT_TRUE(ro1.isReadOnly());
  EXPECT_EQ(kReadOnlyFilesystem, ro1.mkdir("/bar"));
}

TYPED_TEST_P(FsMountTest, ReadWriteDisabledReadOnly) {
  Filesystem& fs = this->fs();
  Mount rw1 = fs.mount();
  ASSERT_TRUE(rw1.ok());
  EXPECT_TRUE(fs.isMounted());
  EXPECT_TRUE(fs.isInUse());
  EXPECT_FALSE(rw1.isReadOnly());
  EXPECT_EQ(kOk, rw1.mkdir("/foo"));

  fs.setMountingPolicy(Filesystem::kMountDisabled);
  Mount disabled = fs.mount();
  ASSERT_FALSE(disabled.ok());
  EXPECT_EQ(kNotMounted, disabled.mkdir("/bar"));

  fs.setMountingPolicy(Filesystem::kMountReadWrite);
  Mount rw2 = fs.mount();
  ASSERT_TRUE(rw2.ok());
  EXPECT_FALSE(rw2.isReadOnly());
  EXPECT_EQ(kOk, rw2.mkdir("/baz"));

  // Re-check that the ro is still read-only.
  EXPECT_EQ(kNotMounted, disabled.status());
  EXPECT_EQ(kNotMounted, disabled.mkdir("/bar"));
}

REGISTER_TYPED_TEST_SUITE_P(FsMountTest, LazyMountEagerUnmount,
                            LazyMountLazyUnmount, UnmountPolicySwitched,
                            ForcedUnmount, ReadWriteReadOnlyReadWrite,
                            ReadWriteDisabledReadOnly);

}  // namespace roo_io
