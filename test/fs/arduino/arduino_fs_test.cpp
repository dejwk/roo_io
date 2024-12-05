#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "fakefs_arduino.h"
#include "roo_io/fs/arduino/arduino_sdfs.h"

namespace roo_io {

void PrintTo(Status s, std::ostream* os) { *os << StatusAsString(s); }

class ArduinoFs : public testing::Test {
 public:
  ArduinoFs() : fakefs_(), sd_(fakefs_), fs_(sd_), mount_(fs_.mount()) {}

  void CreateTextFile(const char* path, const char* contents) {
    ASSERT_EQ(kOk, fakefs::CreateTextFile(fakefs_, path, contents));
  }

  fakefs::FakeFs& fake() { return fakefs_; }
  Mount& mount() { return mount_; }

  fakefs::FakeFs fakefs_;
  fakefs::FakeArduinoSdFsImpl sd_;
  fakefs::FakeArduinoSdFs fs_;
  Mount mount_;
};

TEST_F(ArduinoFs, StatExistingDir) {
  CreateTextFile("/a/t.txt", "foo");

  Stat s = mount().stat("/a");
  EXPECT_EQ(kOk, s.status());
  EXPECT_FALSE(s.isFile());
  EXPECT_TRUE(s.isDirectory());
  EXPECT_EQ(0, s.size());
}

TEST_F(ArduinoFs, StatNonExistentDir) {
  CreateTextFile("/a/t.txt", "foo");

  Stat s = mount().stat("/b");
  EXPECT_EQ(kNotFound, s.status());
  EXPECT_FALSE(s.isFile());
  EXPECT_FALSE(s.isDirectory());
  EXPECT_EQ(0, s.size());
}

TEST_F(ArduinoFs, StatExistingFile) {
  CreateTextFile("/a/t.txt", "foo");

  Stat s = mount().stat("/a/t.txt");
  EXPECT_EQ(kOk, s.status());
  EXPECT_TRUE(s.isFile());
  EXPECT_FALSE(s.isDirectory());
  EXPECT_EQ(3, s.size());
}

TEST_F(ArduinoFs, StatNonExistentFile) {
  CreateTextFile("/a/t.txt", "foo");

  Stat s = mount().stat("/a/t.txt/foo");
  EXPECT_EQ(kNotFound, s.status());
  EXPECT_FALSE(s.isFile());
  EXPECT_FALSE(s.isDirectory());
  EXPECT_EQ(0, s.size());
}

TEST_F(ArduinoFs, SuccessfulRemove) {
  CreateTextFile("/a/t1.txt", "foo");
  CreateTextFile("/a/t2.txt", "bar");

  EXPECT_EQ(kOk, mount().remove("/a/t1.txt"));
  EXPECT_EQ(kNotFound, fake().stat("/a/t1.txt").status);

  // Should not remove more than expected.
  EXPECT_EQ(fakefs::StatResult::kFile, fake().stat("/a/t2.txt").type);
  EXPECT_EQ(fakefs::StatResult::kDir, fake().stat("/a").type);
}

TEST_F(ArduinoFs, UnsuccessfulRemove) {
  CreateTextFile("/a/t.txt", "foo");

  EXPECT_EQ(kNotFile, mount().remove("/a"));
  EXPECT_EQ(kNotFound, mount().remove("/b"));
  EXPECT_EQ(kNotFound, mount().remove("/a/t.txt/foo"));
}

TEST_F(ArduinoFs, SuccessfulMkdir) {
  EXPECT_EQ(kOk, mount().mkdir("/a"));
  EXPECT_EQ(kOk, mount().mkdir("/a/LongDirectorySubname"));
  EXPECT_EQ(kOk, mount().mkdir("/a/LongDirectorySubname/3"));

  EXPECT_EQ(fakefs::StatResult::kDir, fake().stat("/a").type);
  EXPECT_EQ(fakefs::StatResult::kDir,
            fake().stat("/a/LongDirectorySubname").type);
  EXPECT_EQ(fakefs::StatResult::kDir,
            fake().stat("/a/LongDirectorySubname/3").type);
}

TEST_F(ArduinoFs, UnsuccessfulMkdir) {
  CreateTextFile("/a/b/c/t.txt", "foo");
  EXPECT_EQ(kDirectoryExists, mount().mkdir("/a"));
  EXPECT_EQ(kDirectoryExists, mount().mkdir("/a/b"));
  EXPECT_EQ(kDirectoryExists, mount().mkdir("/a/b/c"));
  EXPECT_EQ(kFileExists, mount().mkdir("/a/b/c/t.txt"));
  EXPECT_EQ(kNotDirectory, mount().mkdir("/a/b/c/t.txt/bar"));
  EXPECT_EQ(kNotDirectory, mount().mkdir("/a/b/c/t.txt/bar/"));
}

}  // namespace roo_io
