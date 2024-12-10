#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace roo_io {

void PrintTo(Status s, std::ostream* os) { *os << StatusAsString(s); }

template <typename Fs>
class FsTest : public testing::Test {
 public:
  FsTest() : fakefs_(), fs_(fakefs_), mount_(fs_.mount()) {}

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
    auto stream = this->mount().fopenForWrite(file, policy);
    ASSERT_EQ(kOk, stream->status());
    stream->writeFully((const byte*)contents, strlen(contents));
    stream->close();
    ASSERT_EQ(kClosed, stream->status());
  }

  fakefs::FakeFs fakefs_;
  Fs fs_;
  Mount mount_;
};

TYPED_TEST_SUITE_P(FsTest);

TYPED_TEST_P(FsTest, StatExistingDir) {
  this->CreateTextFile("/a/t.txt", "foo");

  Stat s = this->mount().stat("/a");
  EXPECT_EQ(kOk, s.status());
  EXPECT_FALSE(s.isFile());
  EXPECT_TRUE(s.isDirectory());
  EXPECT_EQ(0, s.size());
}

TYPED_TEST_P(FsTest, StatNonExistentDir) {
  this->CreateTextFile("/a/t.txt", "foo");

  Stat s = this->mount().stat("/b");
  EXPECT_EQ(kNotFound, s.status());
  EXPECT_FALSE(s.isFile());
  EXPECT_FALSE(s.isDirectory());
  EXPECT_EQ(0, s.size());
}

TYPED_TEST_P(FsTest, StatExistingFile) {
  this->CreateTextFile("/a/t.txt", "foo");

  Stat s = this->mount().stat("/a/t.txt");
  EXPECT_EQ(kOk, s.status());
  EXPECT_TRUE(s.isFile());
  EXPECT_FALSE(s.isDirectory());
  EXPECT_EQ(3, s.size());
}

TYPED_TEST_P(FsTest, StatNonExistentFile) {
  this->CreateTextFile("/a/t.txt", "foo");

  Stat s = this->mount().stat("/a/bar.txt");
  EXPECT_EQ(kNotFound, s.status());
  EXPECT_FALSE(s.isFile());
  EXPECT_FALSE(s.isDirectory());
  EXPECT_EQ(0, s.size());
}

TYPED_TEST_P(FsTest, StatNonExistentFileWithFilesInPath) {
  this->CreateTextFile("/a/t.txt", "foo");

  Stat s = this->mount().stat("/a/t.txt/foo");
  if (TypeParam::strict) {
    EXPECT_EQ(kNotDirectory, s.status());
  } else {
    EXPECT_TRUE(s.status() == kNotDirectory || s.status() == kNotFound);
  }
  EXPECT_FALSE(s.isFile());
  EXPECT_FALSE(s.isDirectory());
  EXPECT_EQ(0, s.size());
}

TYPED_TEST_P(FsTest, SuccessfulRemove) {
  this->CreateTextFile("/a/t1.txt", "foo");
  this->CreateTextFile("/a/t2.txt", "bar");

  EXPECT_EQ(kOk, this->mount().remove("/a/t1.txt"));
  EXPECT_EQ(kNotFound, this->fake().stat("/a/t1.txt").status);

  // Should not remove more than expected.
  EXPECT_EQ(fakefs::StatResult::kFile, this->fake().stat("/a/t2.txt").type);
  EXPECT_EQ(fakefs::StatResult::kDir, this->fake().stat("/a").type);
}

TYPED_TEST_P(FsTest, UnsuccessfulRemove) {
  this->CreateTextFile("/a/t.txt", "foo");

  EXPECT_EQ(kInvalidPath, this->mount().remove("a"));
  EXPECT_EQ(kNotFile, this->mount().remove("/a"));
  EXPECT_EQ(kNotFound, this->mount().remove("/b"));

  Status s = this->mount().remove("/a/t.txt/foo");
  if (TypeParam::strict) {
    EXPECT_EQ(kNotDirectory, s);
  } else {
    EXPECT_TRUE(s == kNotDirectory || s == kNotFound);
  }
}

TYPED_TEST_P(FsTest, SuccessfulMkdir) {
  EXPECT_EQ(kOk, this->mount().mkdir("/a"));
  EXPECT_EQ(kOk, this->mount().mkdir("/a/LongDirectorySubname"));
  EXPECT_EQ(kOk, this->mount().mkdir("/a/LongDirectorySubname/3"));

  EXPECT_EQ(fakefs::StatResult::kDir, this->fake().stat("/a").type);
  EXPECT_EQ(fakefs::StatResult::kDir,
            this->fake().stat("/a/LongDirectorySubname").type);
  EXPECT_EQ(fakefs::StatResult::kDir,
            this->fake().stat("/a/LongDirectorySubname/3").type);
}

TYPED_TEST_P(FsTest, UnsuccessfulMkdir) {
  this->CreateTextFile("/a/b/c/t.txt", "foo");
  EXPECT_EQ(kInvalidPath, this->mount().mkdir("a"));
  EXPECT_EQ(kDirectoryExists, this->mount().mkdir("/a"));
  EXPECT_EQ(kDirectoryExists, this->mount().mkdir("/a/b"));
  EXPECT_EQ(kDirectoryExists, this->mount().mkdir("/a/b/c"));
  EXPECT_EQ(kFileExists, this->mount().mkdir("/a/b/c/t.txt"));
  EXPECT_EQ(kNotDirectory, this->mount().mkdir("/a/b/c/t.txt/bar"));
  EXPECT_EQ(kNotDirectory, this->mount().mkdir("/a/b/c/t.txt/bar/"));
}

TYPED_TEST_P(FsTest, SuccessfulRmdir) {
  ASSERT_EQ(kOk, this->mount().mkdir("/a"));
  EXPECT_EQ(1, this->fake().root()->dir().entryCount());
  ASSERT_EQ(kOk, this->mount().mkdir("/a/b"));
  EXPECT_EQ(1, this->fake().root()->dir().entryCount());
  EXPECT_EQ(kOk, this->mount().rmdir("/a/b"));
  EXPECT_EQ(1, this->fake().root()->dir().entryCount());
  EXPECT_EQ(kOk, this->mount().rmdir("/a/"));

  EXPECT_EQ(0, this->fake().root()->dir().entryCount());
}

TYPED_TEST_P(FsTest, UnsuccessfulRmdir) {
  this->CreateTextFile("/a/b/c/t.txt", "foo");
  EXPECT_EQ(kInvalidPath, this->mount().rmdir("a"));
  EXPECT_EQ(kDirectoryNotEmpty, this->mount().rmdir("/a"));
  EXPECT_EQ(kDirectoryNotEmpty, this->mount().rmdir("/a/b/c"));
  EXPECT_EQ(kNotDirectory, this->mount().rmdir("/a/b/c/t.txt"));

  Status s = this->mount().rmdir("/a/b/c/t.txt/foo");
  if (TypeParam::strict) {
    EXPECT_EQ(kNotDirectory, s);
  } else {
    EXPECT_TRUE(s == kNotDirectory || s == kNotFound);
  }
}

TYPED_TEST_P(FsTest, SuccessfulRename) {
  this->CreateTextFile("/a/b/c/foo.txt", "foo");

  EXPECT_EQ(kOk,
            this->mount().rename("/a/b/c/foo.txt", "/a/b/c/foo_moved.txt"));
  EXPECT_EQ("foo", fakefs::ReadTextFile(this->fake(), "/a/b/c/foo_moved.txt"));
  EXPECT_EQ(kNotFound, this->fake().stat("/a/b/c/foo.txt").status);
}

TYPED_TEST_P(FsTest, SuccessfulMove) {
  this->CreateTextFile("/a/b/c/foo.txt", "foo");

  EXPECT_EQ(kOk, this->mount().rename("/a/b/c/foo.txt", "/a/b/foo.txt"));
  EXPECT_EQ("foo", fakefs::ReadTextFile(this->fake(), "/a/b/foo.txt"));
  EXPECT_EQ(kNotFound, this->fake().stat("/a/b/c/foo.txt").status);
}

TYPED_TEST_P(FsTest, SuccessfulMoveDir) {
  this->CreateTextFile("/a/b/c/foo.txt", "foo");

  EXPECT_EQ(kOk, this->mount().rename("/a/b/c", "/a/moved"));
  EXPECT_EQ("foo", fakefs::ReadTextFile(this->fake(), "/a/moved/foo.txt"));
  EXPECT_EQ(kNotFound, this->fake().stat("/a/b/c/foo.txt").status);
}

TYPED_TEST_P(FsTest, UnsuccessfulRename) {
  this->CreateTextFile("/a/b/c/foo.txt", "foo");
  this->CreateTextFile("/a/b/d/bar.txt", "bar");

  EXPECT_EQ(kInvalidPath, this->mount().rename("a", "/b"));
  EXPECT_EQ(kInvalidPath, this->mount().rename("/a", "b"));
  EXPECT_EQ(kInvalidPath, this->mount().rename("/a/b", ""));

  EXPECT_EQ(kInvalidPath, this->mount().rename("/a", "/a/b/a"));

  EXPECT_EQ(kNotFound, this->mount().rename("/a/d", "/x"));
  EXPECT_EQ(kNotFound, this->mount().rename("/a/b/c", "/a/b/e/d"));

  Status s = this->mount().rename("/a/b/c/foo.txt/e", "/x");
  if (TypeParam::strict) {
    EXPECT_EQ(kNotDirectory, s);
  } else {
    EXPECT_TRUE(s == kNotDirectory || s == kNotFound);
  }

  EXPECT_EQ(kNotDirectory, this->mount().rename("/a/b/c", "/a/b/d/bar.txt/x"));

  EXPECT_EQ(kFileExists, this->mount().rename("/a/b/c", "/a/b/d/bar.txt"));
  EXPECT_EQ(kDirectoryExists, this->mount().rename("/a/b/c", "/a/b/d"));

  EXPECT_EQ(kFileExists,
            this->mount().rename("/a/b/d/bar.txt", "/a/b/d/bar.txt"));
  EXPECT_EQ(kDirectoryExists, this->mount().rename("/a/b", "/a/b"));
}

TYPED_TEST_P(FsTest, ListEmptyDir) {
  Directory dir = this->mount().opendir("/");
  EXPECT_EQ(kOk, dir.status());
  EXPECT_STREQ("/", dir.path());
  EXPECT_STREQ("", dir.name());
  EXPECT_FALSE(dir.read());
  EXPECT_FALSE(dir.read());
  dir.rewind();
  EXPECT_FALSE(dir.read());
}

TYPED_TEST_P(FsTest, ListOneElemDir) {
  this->CreateTextFile("/a/b/foo.txt", "foo");

  Directory dir = this->mount().opendir("/a/b");
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

TYPED_TEST_P(FsTest, ListDir) {
  this->CreateTextFile("/a/b/foo.txt", "foo");
  this->CreateTextFile("/a/b/c/foo.txt", "foo");
  this->CreateTextFile("/a/b/bar.txt", "bar");
  this->CreateTextFile("/a/b/c/bar.txt", "bar");

  Directory dir = this->mount().opendir("/a/b");
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

TYPED_TEST_P(FsTest, SuccessfullyReadFile) {
  this->CreateTextFile("/a/b/foo.txt", "This is my text file");

  auto stream = this->mount().fopen("/a/b/foo.txt");
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

TYPED_TEST_P(FsTest, UnsuccessfulFopen) {
  this->CreateTextFile("/a/b/foo.txt", "foo");

  EXPECT_EQ(kInvalidPath, this->mount().fopen("a")->status());
  EXPECT_EQ(kNotFound, this->mount().fopen("/a/bar.txt")->status());
  EXPECT_EQ(kNotFile, this->mount().fopen("/a/b")->status());

  Status s = this->mount().fopen("/a/b/foo.txt/c")->status();
  if (TypeParam::strict) {
    EXPECT_EQ(kNotDirectory, s);
  } else {
    EXPECT_TRUE(s == kNotDirectory || s == kNotFound);
  }
}

TYPED_TEST_P(FsTest, SuccessfulCreateFile) {
  this->RecursiveMkDir("/foo/bar");
  const char* contents = "This is my text file";
  this->CreateAndWrite("/foo/bar/foo1.txt", roo_io::kFailIfExists, contents);
  EXPECT_EQ(contents, fakefs::ReadTextFile(this->fake(), "/foo/bar/foo1.txt"));
  this->CreateAndWrite("/foo/bar/foo2.txt", roo_io::kTruncateIfExists,
                       contents);
  EXPECT_EQ(contents, fakefs::ReadTextFile(this->fake(), "/foo/bar/foo2.txt"));
  this->CreateAndWrite("/foo/bar/foo3.txt", roo_io::kAppendIfExists, contents);
  EXPECT_EQ(contents, fakefs::ReadTextFile(this->fake(), "/foo/bar/foo3.txt"));
}

TYPED_TEST_P(FsTest, SuccessfulOverwriteFile) {
  this->CreateTextFile("/a/b/foo.txt", "Previous contents");
  const char* contents = "New contents";
  this->CreateAndWrite("/a/b/foo.txt", roo_io::kTruncateIfExists, contents);
  EXPECT_EQ("New contents", fakefs::ReadTextFile(this->fake(), "/a/b/foo.txt"));
}

TYPED_TEST_P(FsTest, SuccessfulAppendToFile) {
  this->CreateTextFile("/a/b/foo.txt", "Previous contents");
  const char* contents = "New contents";
  this->CreateAndWrite("/a/b/foo.txt", roo_io::kAppendIfExists, contents);
  EXPECT_EQ("Previous contentsNew contents",
            fakefs::ReadTextFile(this->fake(), "/a/b/foo.txt"));
}

TYPED_TEST_P(FsTest, ReadSeekAndSkip) {
  this->CreateTextFile("/a/b/foo.txt", "This is my text file");

  auto stream = this->mount().fopen("/a/b/foo.txt");
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

TYPED_TEST_P(FsTest, ReadStressTest) {
  size_t size = 1024 * 1024 + 17;
  std::unique_ptr<byte[]> contents(new byte[size]);
  for (size_t i = 0; i < size; ++i) contents[i] = (byte)(rand() % 256);
  this->RecursiveMkDir("/foo/bar");
  {
    std::unique_ptr<OutputStream> out =
        this->mount().fopenForWrite("/foo/bar/foo.test", kFailIfExists);
    ASSERT_EQ(size, out->writeFully(contents.get(), size));
    out->close();
    ASSERT_EQ(kClosed, out->status());
  }

  std::unique_ptr<MultipassInputStream> in =
      this->mount().fopen("/foo/bar/foo.test");
  byte buf[20000];
  ASSERT_EQ(kOk, in->status());
  size_t pos = 0;
  for (size_t i = 0; i < 10000; i++) {
    int cnt = rand() % 20000;
    size_t read = in->read(buf, cnt);
    ASSERT_LE(read, cnt);
    for (size_t j = 0; j < read; ++j) {
      ASSERT_EQ(buf[j], contents[pos + j]);
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

REGISTER_TYPED_TEST_SUITE_P(FsTest, StatExistingDir, StatNonExistentDir,
                            StatExistingFile, StatNonExistentFile,
                            StatNonExistentFileWithFilesInPath,
                            SuccessfulRemove, UnsuccessfulRemove,
                            SuccessfulMkdir, UnsuccessfulMkdir, SuccessfulRmdir,
                            UnsuccessfulRmdir, SuccessfulRename, SuccessfulMove,
                            SuccessfulMoveDir, UnsuccessfulRename, ListEmptyDir,
                            ListOneElemDir, ListDir, SuccessfullyReadFile,
                            UnsuccessfulFopen, SuccessfulCreateFile,
                            SuccessfulOverwriteFile, SuccessfulAppendToFile,
                            ReadSeekAndSkip, ReadStressTest);

}  // namespace roo_io
