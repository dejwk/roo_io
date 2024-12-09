#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "fakefs.h"

namespace roo_io {
namespace fakefs {

TEST(FakeFs, EmptyOnCreate) {
  FakeFs fs;
  Entry* entry;
  ASSERT_EQ(kOk, fs.findEntryByPath("/", &entry));
  ASSERT_NE(nullptr, entry);
  EXPECT_TRUE(entry->isDir());
  EXPECT_EQ(0, entry->dir().entryCount());
  EXPECT_EQ(0, fs.root()->dir().entryCount());
}

TEST(FakeFs, NonexistentPaths) {
  FakeFs fs;
  Entry* entry;
  EXPECT_EQ(kInvalidPath, fs.findEntryByPath("", &entry));
  EXPECT_EQ(kInvalidPath, fs.findEntryByPath("aaa", &entry));
  EXPECT_EQ(kNotFound, fs.findEntryByPath("/a", &entry));
  EXPECT_EQ(kNotFound, fs.findEntryByPath("/b/", &entry));
  EXPECT_EQ(kNotFound, fs.findEntryByPath("/c/d", &entry));
}

TEST(FakeFs, CreateAndRemoveDirectories) {
  FakeFs fs;
  Entry* a;
  Entry* b;
  Entry* c;
  EXPECT_EQ(kOk, fs.root()->dir().mkdir(fs.root(), "a", &a));
  EXPECT_EQ(kOk, fs.root()->dir().mkdir(fs.root(), "b", &b));
  EXPECT_EQ(kOk, a->dir().mkdir(fs.root(), "c", &c));

  Entry* entry;
  ASSERT_EQ(kOk, fs.findEntryByPath("/a", &entry));
  EXPECT_TRUE(entry->isDir());
  EXPECT_EQ(entry, a);
  ASSERT_EQ(kOk, fs.findEntryByPath("/a/c", &entry));
  EXPECT_TRUE(entry->isDir());
  EXPECT_EQ(entry, c);
  EXPECT_EQ(2, fs.root()->dir().entryCount());
  EXPECT_EQ(1, a->dir().entryCount());
  EXPECT_EQ(kDirectoryNotEmpty, fs.root()->dir().rmdir("a"));
  EXPECT_EQ(kOk, a->dir().rmdir("c"));
  EXPECT_EQ(kOk, fs.root()->dir().rmdir("a"));
}

TEST(FakeFs, CreateAndReadTextFile) {
  FakeFs fs;
  EXPECT_EQ(kOk, CreateTextFile(fs, "/a/b/c.txt", "My contents"));
  EXPECT_TRUE(fs.stat("/a/b/c.txt").type == StatResult::kFile);
  EXPECT_EQ(ReadTextFile(fs, "/a/b/c.txt"), std::string("My contents"));
  EXPECT_EQ(kDirectoryExists, CreateTextFile(fs, "/a/b", "My contents"));
  EXPECT_EQ(kOk, CreateTextFile(fs, "/a/b.txt", "b"));
  EXPECT_EQ(ReadTextFile(fs, "/a/b.txt"), std::string("b"));
  EXPECT_EQ(kInvalidPath, CreateTextFile(fs, "b", "foo"));
}

}  // namespace fakefs
}  // namespace roo_io