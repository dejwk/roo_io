#include <cstdlib>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "roo_io/fs/filesystem.h"
#include "roo_io/fs/posix/posix_mount.h"

namespace roo_io {
namespace {

class TempDirectory {
 public:
  TempDirectory() {
    char path[] = "/tmp/roo_io_posix_dirXXXXXX";
    root_ = mkdtemp(path);
  }

  ~TempDirectory() {
    std::error_code error;
    std::filesystem::remove_all(root_, error);
  }

  const std::string& path() const { return root_; }

 private:
  std::string root_;
};

class PosixTestFilesystem : public Filesystem {
 public:
  explicit PosixTestFilesystem(std::string root) : root_(std::move(root)) {}

  MediaPresence checkMediaPresence() override { return kMediaPresent; }

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override {
    return MountImpl::Mounted(std::unique_ptr<MountImpl>(
        new PosixMountImpl(root_.c_str(), true, std::move(unmount_fn))));
  }

  void unmountImpl() override {}

 private:
  std::string root_;
};

TEST(PosixDirectoryTest, OmitsSelfAndParentEntries) {
  TempDirectory root;
  ASSERT_TRUE(std::filesystem::create_directory(root.path() + "/child"));
  PosixTestFilesystem filesystem(root.path());
  Mount mount = filesystem.mount();
  ASSERT_TRUE(mount.ok());

  Directory directory = mount.opendir("/");
  ASSERT_TRUE(directory.read());
  EXPECT_STREQ("child", directory.entry().name());
  EXPECT_TRUE(directory.entry().isDirectory());
  EXPECT_FALSE(directory.read());
  EXPECT_EQ(kEndOfStream, directory.status());
}

}  // namespace
}  // namespace roo_io
