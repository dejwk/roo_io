#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "fakefs_reference.h"

namespace roo_io {

void PrintTo(Status s, std::ostream* os) { *os << StatusAsString(s); }

class ReferenceFs : public testing::Test {
 public:
  ReferenceFs() : fakefs_(), fs_(fakefs_), mount_(fs_.mount()) {}

  void CreateTextFile(const char* path, const char* contents) {
    ASSERT_EQ(kOk, fakefs::CreateTextFile(fakefs_, path, contents));
  }

  fakefs::FakeFs& fake() { return fakefs_; }
  Mount& mount() { return mount_; }

  fakefs::FakeFs fakefs_;
  fakefs::FakeReferenceFs fs_;
  Mount mount_;
};

TEST_F(ReferenceFs, StatExistingDir) {
  CreateTextFile("/a/t.txt", "foo");

  Stat s = mount().stat("/a");
  EXPECT_EQ(kOk, s.status());
  EXPECT_FALSE(s.isFile());
  EXPECT_TRUE(s.isDirectory());
  EXPECT_EQ(0, s.size());
}

TEST_F(ReferenceFs, StatNonExistentDir) {
  CreateTextFile("/a/t.txt", "foo");

  Stat s = mount().stat("/b");
  EXPECT_EQ(kNotFound, s.status());
  EXPECT_FALSE(s.isFile());
  EXPECT_FALSE(s.isDirectory());
  EXPECT_EQ(0, s.size());
}

TEST_F(ReferenceFs, StatExistingFile) {
  CreateTextFile("/a/t.txt", "foo");

  Stat s = mount().stat("/a/t.txt");
  EXPECT_EQ(kOk, s.status());
  EXPECT_TRUE(s.isFile());
  EXPECT_FALSE(s.isDirectory());
  EXPECT_EQ(3, s.size());
}

TEST_F(ReferenceFs, StatNonExistentFile) {
  CreateTextFile("/a/t.txt", "foo");

  Stat s = mount().stat("/a/bar.txt");
  EXPECT_EQ(kNotFound, s.status());
  EXPECT_FALSE(s.isFile());
  EXPECT_FALSE(s.isDirectory());
  EXPECT_EQ(0, s.size());
}

TEST_F(ReferenceFs, StatNonExistentFileWithFilesInPath) {
  CreateTextFile("/a/t.txt", "foo");

  Stat s = mount().stat("/a/t.txt/foo");
  EXPECT_EQ(kNotDirectory, s.status());
  EXPECT_FALSE(s.isFile());
  EXPECT_FALSE(s.isDirectory());
  EXPECT_EQ(0, s.size());
}

TEST_F(ReferenceFs, SuccessfulRemove) {
  CreateTextFile("/a/t1.txt", "foo");
  CreateTextFile("/a/t2.txt", "bar");

  EXPECT_EQ(kOk, mount().remove("/a/t1.txt"));
  EXPECT_EQ(kNotFound, fake().stat("/a/t1.txt").status);

  // Should not remove more than expected.
  EXPECT_EQ(fakefs::StatResult::kFile, fake().stat("/a/t2.txt").type);
  EXPECT_EQ(fakefs::StatResult::kDir, fake().stat("/a").type);
}

TEST_F(ReferenceFs, UnsuccessfulRemove) {
  CreateTextFile("/a/t.txt", "foo");

  EXPECT_EQ(kInvalidPath, mount().remove("a"));
  EXPECT_EQ(kNotFile, mount().remove("/a"));
  EXPECT_EQ(kNotFound, mount().remove("/b"));
  EXPECT_EQ(kNotDirectory, mount().remove("/a/t.txt/foo"));
}

TEST_F(ReferenceFs, SuccessfulMkdir) {
  EXPECT_EQ(kOk, mount().mkdir("/a"));
  EXPECT_EQ(kOk, mount().mkdir("/a/LongDirectorySubname"));
  EXPECT_EQ(kOk, mount().mkdir("/a/LongDirectorySubname/3"));

  EXPECT_EQ(fakefs::StatResult::kDir, fake().stat("/a").type);
  EXPECT_EQ(fakefs::StatResult::kDir,
            fake().stat("/a/LongDirectorySubname").type);
  EXPECT_EQ(fakefs::StatResult::kDir,
            fake().stat("/a/LongDirectorySubname/3").type);
}

TEST_F(ReferenceFs, UnsuccessfulMkdir) {
  CreateTextFile("/a/b/c/t.txt", "foo");
  EXPECT_EQ(kInvalidPath, mount().mkdir("a"));
  EXPECT_EQ(kDirectoryExists, mount().mkdir("/a"));
  EXPECT_EQ(kDirectoryExists, mount().mkdir("/a/b"));
  EXPECT_EQ(kDirectoryExists, mount().mkdir("/a/b/c"));
  EXPECT_EQ(kFileExists, mount().mkdir("/a/b/c/t.txt"));
  EXPECT_EQ(kNotDirectory, mount().mkdir("/a/b/c/t.txt/bar"));
  EXPECT_EQ(kNotDirectory, mount().mkdir("/a/b/c/t.txt/bar/"));
}

TEST_F(ReferenceFs, SuccessfulRmdir) {
  ASSERT_EQ(kOk, mount().mkdir("/a"));
  EXPECT_EQ(1, fake().root()->dir().entryCount());
  ASSERT_EQ(kOk, mount().mkdir("/a/b"));
  EXPECT_EQ(1, fake().root()->dir().entryCount());
  EXPECT_EQ(kOk, mount().rmdir("/a/b"));
  EXPECT_EQ(1, fake().root()->dir().entryCount());
  EXPECT_EQ(kOk, mount().rmdir("/a/"));

  EXPECT_EQ(0, fake().root()->dir().entryCount());
}

TEST_F(ReferenceFs, UnsuccessfulRmdir) {
  CreateTextFile("/a/b/c/t.txt", "foo");
  EXPECT_EQ(kInvalidPath, mount().rmdir("a"));
  EXPECT_EQ(kDirectoryNotEmpty, mount().rmdir("/a"));
  EXPECT_EQ(kDirectoryNotEmpty, mount().rmdir("/a/b/c"));
  EXPECT_EQ(kNotDirectory, mount().rmdir("/a/b/c/t.txt"));
  EXPECT_EQ(kNotDirectory, mount().rmdir("/a/b/c/t.txt/foo"));
}

TEST_F(ReferenceFs, SuccessfulRename) {
  CreateTextFile("/a/b/c/foo.txt", "foo");

  EXPECT_EQ(kOk, mount().rename("/a/b/c/foo.txt", "/a/b/c/foo_moved.txt"));
  EXPECT_EQ("foo", fakefs::ReadTextFile(fake(), "/a/b/c/foo_moved.txt"));
  EXPECT_EQ(kNotFound, fake().stat("/a/b/c/foo.txt").status);
}

TEST_F(ReferenceFs, SuccessfulMove) {
  CreateTextFile("/a/b/c/foo.txt", "foo");

  EXPECT_EQ(kOk, mount().rename("/a/b/c/foo.txt", "/a/b/foo.txt"));
  EXPECT_EQ("foo", fakefs::ReadTextFile(fake(), "/a/b/foo.txt"));
  EXPECT_EQ(kNotFound, fake().stat("/a/b/c/foo.txt").status);
}

TEST_F(ReferenceFs, SuccessfulMoveDir) {
  CreateTextFile("/a/b/c/foo.txt", "foo");

  EXPECT_EQ(kOk, mount().rename("/a/b/c", "/a/moved"));
  EXPECT_EQ("foo", fakefs::ReadTextFile(fake(), "/a/moved/foo.txt"));
  EXPECT_EQ(kNotFound, fake().stat("/a/b/c/foo.txt").status);
}

TEST_F(ReferenceFs, UnsuccessfulRename) {
  CreateTextFile("/a/b/c/foo.txt", "foo");
  CreateTextFile("/a/b/d/bar.txt", "bar");

  EXPECT_EQ(kInvalidPath, mount().rename("a", "/b"));
  EXPECT_EQ(kInvalidPath, mount().rename("/a", "b"));
  EXPECT_EQ(kInvalidPath, mount().rename("/a/b", ""));

  EXPECT_EQ(kInvalidPath, mount().rename("/a", "/a/b/a"));

  EXPECT_EQ(kNotFound, mount().rename("/a/d", "/x"));
  EXPECT_EQ(kNotFound, mount().rename("/a/b/c", "/a/b/e/d"));

  EXPECT_EQ(kNotDirectory, mount().rename("/a/b/c/foo.txt/e", "/x"));
  EXPECT_EQ(kNotDirectory, mount().rename("/a/b/c", "/a/b/d/bar.txt/x"));

  EXPECT_EQ(kFileExists, mount().rename("/a/b/c", "/a/b/d/bar.txt"));
  EXPECT_EQ(kDirectoryExists, mount().rename("/a/b/c", "/a/b/d"));

  EXPECT_EQ(kFileExists, mount().rename("/a/b/d/bar.txt", "/a/b/d/bar.txt"));
  EXPECT_EQ(kDirectoryExists, mount().rename("/a/b", "/a/b"));
}

TEST_F(ReferenceFs, ListEmptyDir) {
  Directory dir = mount().opendir("/");
  EXPECT_EQ(kOk, dir.status());
  EXPECT_STREQ("/", dir.path());
  EXPECT_STREQ("", dir.name());
  Directory::Entry e = dir.read();
  EXPECT_TRUE(e.done());
  e = dir.read();
  EXPECT_TRUE(e.done());
  dir.rewind();
  e = dir.read();
  EXPECT_TRUE(e.done());
}

TEST_F(ReferenceFs, ListOneElemDir) {
  CreateTextFile("/a/b/foo.txt", "foo");

  Directory dir = mount().opendir("/a/b");
  EXPECT_EQ(kOk, dir.status());
  EXPECT_STREQ("/a/b", dir.path());
  EXPECT_STREQ("b", dir.name());
  Directory::Entry e = dir.read();
  EXPECT_FALSE(e.done());
  EXPECT_EQ(kOk, dir.status());
  EXPECT_STREQ("foo.txt", e.name());
  EXPECT_STREQ("/a/b/foo.txt", e.path());
  EXPECT_FALSE(e.isDirectory());
  e = dir.read();
  EXPECT_TRUE(e.done());
  EXPECT_EQ(kEndOfStream, dir.status());
  dir.rewind();
  e = dir.read();
  EXPECT_FALSE(e.done());
  EXPECT_EQ(kOk, dir.status());
  EXPECT_STREQ("foo.txt", e.name());
  EXPECT_STREQ("/a/b/foo.txt", e.path());
  EXPECT_FALSE(e.isDirectory());
  e = dir.read();
  EXPECT_TRUE(e.done());
  EXPECT_EQ(kEndOfStream, dir.status());
  dir.rewind();
  EXPECT_EQ(kOk, dir.status());
  dir.close();
  EXPECT_EQ(kClosed, dir.status());
  e = dir.read();
  EXPECT_TRUE(e.done());

}

}  // namespace roo_io
