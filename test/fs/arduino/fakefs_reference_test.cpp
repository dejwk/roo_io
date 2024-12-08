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

  void RecursiveMkDir(const char* path) {
    ASSERT_EQ(kOk, fakefs::RecursiveMkDir(fakefs_, path));
  }

  fakefs::FakeFs& fake() { return fakefs_; }
  Mount& mount() { return mount_; }

  void CreateAndWrite(const char* file, FileUpdatePolicy policy,
                      const char* contents) {
    auto stream = mount().fopenForWrite(file, policy);
    ASSERT_EQ(kOk, stream->status());
    stream->writeFully((const byte*)contents, strlen(contents));
    stream->close();
    ASSERT_EQ(kClosed, stream->status());
  }

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
  EXPECT_FALSE(dir.read());
  EXPECT_FALSE(dir.read());
  dir.rewind();
  EXPECT_FALSE(dir.read());
}

TEST_F(ReferenceFs, ListOneElemDir) {
  CreateTextFile("/a/b/foo.txt", "foo");

  Directory dir = mount().opendir("/a/b");
  EXPECT_EQ(kOk, dir.status());
  EXPECT_STREQ("/a/b", dir.path());
  EXPECT_STREQ("b", dir.name());
  ASSERT_TRUE(dir.read());
  EXPECT_EQ(kOk, dir.status());
  EXPECT_STREQ("foo.txt", dir.entry().name());
  EXPECT_STREQ("/a/b/foo.txt", dir.entry().path());
  EXPECT_FALSE(dir.entry().isDirectory());
  EXPECT_FALSE(dir.read());
  EXPECT_EQ(kEndOfStream, dir.status());
  dir.rewind();
  EXPECT_TRUE(dir.read());
  EXPECT_EQ(kOk, dir.status());
  EXPECT_STREQ("foo.txt", dir.entry().name());
  EXPECT_STREQ("/a/b/foo.txt", dir.entry().path());
  EXPECT_FALSE(dir.entry().isDirectory());
  EXPECT_FALSE(dir.read());
  EXPECT_EQ(kEndOfStream, dir.status());
  dir.rewind();
  EXPECT_EQ(kOk, dir.status());
  dir.close();
  EXPECT_EQ(kClosed, dir.status());
  EXPECT_FALSE(dir.read());
}

TEST_F(ReferenceFs, ListDir) {
  CreateTextFile("/a/b/foo.txt", "foo");
  CreateTextFile("/a/b/c/foo.txt", "foo");
  CreateTextFile("/a/b/bar.txt", "bar");
  CreateTextFile("/a/b/c/bar.txt", "bar");

  Directory dir = mount().opendir("/a/b");
  EXPECT_EQ(kOk, dir.status());
  EXPECT_STREQ("/a/b", dir.path());
  EXPECT_STREQ("b", dir.name());

  EXPECT_TRUE(dir.read());
  EXPECT_EQ(kOk, dir.status());
  EXPECT_STREQ("foo.txt", dir.entry().name());
  EXPECT_STREQ("/a/b/foo.txt", dir.entry().path());
  EXPECT_FALSE(dir.entry().isDirectory());
  EXPECT_TRUE(dir.read());
  EXPECT_EQ(kOk, dir.status());
  EXPECT_STREQ("c", dir.entry().name());
  EXPECT_STREQ("/a/b/c", dir.entry().path());
  EXPECT_TRUE(dir.entry().isDirectory());
  EXPECT_TRUE(dir.read());
  EXPECT_EQ(kOk, dir.status());
  EXPECT_STREQ("bar.txt", dir.entry().name());
  EXPECT_STREQ("/a/b/bar.txt", dir.entry().path());
  EXPECT_FALSE(dir.entry().isDirectory());
  EXPECT_FALSE(dir.read());
  EXPECT_EQ(kEndOfStream, dir.status());

  dir.rewind();
  EXPECT_TRUE(dir.read());
  EXPECT_EQ(kOk, dir.status());
  EXPECT_STREQ("foo.txt", dir.entry().name());
  EXPECT_STREQ("/a/b/foo.txt", dir.entry().path());
  dir.close();
  EXPECT_EQ(kClosed, dir.status());
}

TEST_F(ReferenceFs, SuccessfullyReadFile) {
  CreateTextFile("/a/b/foo.txt", "This is my text file");

  auto stream = mount().fopen("/a/b/foo.txt");
  ASSERT_EQ(kOk, stream->status());
  std::string contents;
  while (stream->status() == kOk) {
    byte buf[5];
    size_t read = stream->read(buf, 5);
    contents.append((const char*)buf, read);
  }
  ASSERT_EQ(kEndOfStream, stream->status());
  EXPECT_EQ(contents, "This is my text file");
}

TEST_F(ReferenceFs, UnsuccessfulFopen) {
  CreateTextFile("/a/b/foo.txt", "foo");

  EXPECT_EQ(kInvalidPath, mount().fopen("a")->status());
  EXPECT_EQ(kNotFound, mount().fopen("/a/bar.txt")->status());
  EXPECT_EQ(kNotFile, mount().fopen("/a/b")->status());
  EXPECT_EQ(kNotDirectory, mount().fopen("/a/b/foo.txt/c")->status());
}

TEST_F(ReferenceFs, SuccessfulCreateFile) {
  RecursiveMkDir("/foo/bar");
  const char* contents = "This is my text file";
  CreateAndWrite("/foo/bar/foo1.txt", roo_io::kFailIfExists, contents);
  EXPECT_EQ(contents, fakefs::ReadTextFile(fake(), "/foo/bar/foo1.txt"));
  CreateAndWrite("/foo/bar/foo2.txt", roo_io::kTruncateIfExists, contents);
  EXPECT_EQ(contents, fakefs::ReadTextFile(fake(), "/foo/bar/foo2.txt"));
  CreateAndWrite("/foo/bar/foo3.txt", roo_io::kAppendIfExists, contents);
  EXPECT_EQ(contents, fakefs::ReadTextFile(fake(), "/foo/bar/foo3.txt"));
}

TEST_F(ReferenceFs, SuccessfulOverwriteFile) {
  CreateTextFile("/a/b/foo.txt", "Previous contents");
  const char* contents = "New contents";
  CreateAndWrite("/a/b/foo.txt", roo_io::kTruncateIfExists, contents);
  EXPECT_EQ("New contents", fakefs::ReadTextFile(fake(), "/a/b/foo.txt"));
}

TEST_F(ReferenceFs, SuccessfulAppendToFile) {
  CreateTextFile("/a/b/foo.txt", "Previous contents");
  const char* contents = "New contents";
  CreateAndWrite("/a/b/foo.txt", roo_io::kAppendIfExists, contents);
  EXPECT_EQ("Previous contentsNew contents",
            fakefs::ReadTextFile(fake(), "/a/b/foo.txt"));
}

TEST_F(ReferenceFs, ReadSeekAndSkip) {
  CreateTextFile("/a/b/foo.txt", "This is my text file");

  auto stream = mount().fopen("/a/b/foo.txt");
  ASSERT_EQ(kOk, stream->status());
  EXPECT_EQ(0, stream->position());
  byte buf[5];
  EXPECT_EQ(5, stream->readFully(buf, 5));
  ASSERT_EQ(kOk, stream->status());
  EXPECT_EQ(5, stream->position());
  stream->skip(10);
  ASSERT_EQ(kOk, stream->status());
  EXPECT_EQ(15, stream->position());
  stream->skip(10);
  ASSERT_EQ(kEndOfStream, stream->status());
  EXPECT_EQ(20, stream->position());
  stream->seek(30);
  ASSERT_EQ(kOk, stream->status());
  EXPECT_EQ(30, stream->position());
  EXPECT_EQ(0, stream->readFully(buf, 5));
  ASSERT_EQ(kEndOfStream, stream->status());
  stream->seek(17);
  ASSERT_EQ(kOk, stream->status());
  EXPECT_EQ(17, stream->position());
  EXPECT_EQ(3, stream->readFully(buf, 5));
  ASSERT_EQ(kEndOfStream, stream->status());
  EXPECT_EQ("ile", std::string((const char*)buf, 3));
}

TEST_F(ReferenceFs, ReadStressTest) {
  size_t size = 1024 * 1024 + 17;
  std::unique_ptr<byte[]> contents(new byte[size]);
  for (size_t i = 0; i < size; ++i) contents[i] = rand() % 256;
  RecursiveMkDir("/foo/bar");
  {
    std::unique_ptr<OutputStream> out = mount().fopenForWrite("/foo/bar/foo.test", kFailIfExists);
    ASSERT_EQ(size, out->writeFully(contents.get(), size));
    out->close();
    ASSERT_EQ(kClosed, out->status());
  }

  std::unique_ptr<MultipassInputStream> in = mount().fopen("/foo/bar/foo.test");
  byte buf[20000];
  ASSERT_EQ(kOk, in->status());
  size_t pos = 0;
  for (int i = 0; i < 10000; i++) {
    int cnt = rand() % 20000;
    size_t read = in->read(buf, cnt);
    ASSERT_TRUE(read <= cnt);
    for (int i = 0; i < read; ++i) {
      ASSERT_EQ(buf[i], contents[pos + i]);
    }
    pos += read;
    if (rand() % 100 > 10) {
      uint64_t newpos = rand() % (3 * size);
      in->seek(newpos);
      ASSERT_EQ(kOk, in->status());
      pos = newpos;
    }
    if (rand() % 100 > 10) {
      uint64_t offset = rand() % 50000;
      in->skip(offset);
      if (pos + offset > size) {
        ASSERT_EQ(kEndOfStream, in->status());
        pos = size;
      } else {
        ASSERT_EQ(kOk, in->status());
        pos += offset;
      }
    }
  }
}

}  // namespace roo_io
