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

TYPED_TEST_P(FsMountTest, DirectoryKeepsMountAlive) {
  ASSERT_EQ(kOk, CreateTextFile(this->fake(), "/dir/foo.txt", "Foo"));
  ASSERT_EQ(kOk, CreateTextFile(this->fake(), "/dir/bar.txt", "Bar"));
  Filesystem& fs = this->fs();
  Mount m = fs.mount();
  ASSERT_TRUE(fs.isMounted());

  auto dir = m.opendir("/dir");
  ASSERT_EQ(kOk, dir.status());
  m = Mount();
  ASSERT_TRUE(fs.isMounted());
  ASSERT_TRUE(dir.isOpen());
  ASSERT_TRUE(dir.read());
  EXPECT_STREQ("foo.txt", dir.entry().name());
  ASSERT_TRUE(dir.read());
  EXPECT_STREQ("bar.txt", dir.entry().name());
  ASSERT_FALSE(dir.read());
  ASSERT_TRUE(fs.isMounted());
  dir.rewind();
  ASSERT_TRUE(dir.read());
  EXPECT_STREQ("foo.txt", dir.entry().name());
  ASSERT_TRUE(fs.isMounted());
  dir.close();
  EXPECT_FALSE(dir.isOpen());
  EXPECT_EQ(kClosed, dir.status());
  ASSERT_FALSE(fs.isMounted());
}

TYPED_TEST_P(FsMountTest, InputStreamKeepsMountAlive) {
  ASSERT_EQ(kOk, CreateTextFile(this->fake(), "/foo.txt", "Foo"));
  Filesystem& fs = this->fs();
  Mount m = fs.mount();
  ASSERT_TRUE(fs.isMounted());

  auto in = m.fopen("/foo.txt");
  ASSERT_EQ(kOk, in->status());
  m = Mount();
  ASSERT_TRUE(fs.isMounted());
  char buf[4];
  EXPECT_EQ(3, in->readFully((byte*)buf, 4));
  EXPECT_THAT(std::vector<char>(buf, buf + 3),
              testing::ElementsAre('F', 'o', 'o'));
  EXPECT_TRUE(fs.isMounted());
  in->close();
  EXPECT_FALSE(fs.isMounted());
}

TYPED_TEST_P(FsMountTest, OutputStreamKeepsMountAlive) {
  Filesystem& fs = this->fs();
  Mount m = fs.mount();
  ASSERT_TRUE(fs.isMounted());

  auto out = m.fopenForWrite("/foo.txt", kTruncateIfExists);
  ASSERT_EQ(kOk, out->status());
  m = Mount();
  ASSERT_TRUE(fs.isMounted());
  ASSERT_EQ(4, out->writeFully((const byte*)"foo", 4));
  ASSERT_EQ(kOk, out->status());
  EXPECT_TRUE(fs.isMounted());
  out->close();
  EXPECT_FALSE(fs.isMounted());
}

REGISTER_TYPED_TEST_SUITE_P(FsMountTest, LazyMountEagerUnmount,
                            LazyMountLazyUnmount, UnmountPolicySwitched,
                            ForcedUnmount, ReadWriteReadOnlyReadWrite,
                            ReadWriteDisabledReadOnly,
                            InputStreamKeepsMountAlive,
                            OutputStreamKeepsMountAlive,
                            DirectoryKeepsMountAlive);

}  // namespace roo_io
