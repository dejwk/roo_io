#include "roo_io/fs/esp32/littlefs.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include "gtest/gtest.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"

namespace roo_io {
namespace {

TEST(LittlefsTest, ReadsFileFromFakeEsp32Root) {
  const std::string previous = FakeEsp32().fs_root();
  char path[] = "/tmp/roo_io_littlefs_testXXXXXX";
  const char* root = mkdtemp(path);
  ASSERT_NE(nullptr, root);
  const std::string directory = std::string(root) + "/littlefs";
  ASSERT_TRUE(std::filesystem::create_directory(directory));
  std::ofstream(directory + "/config.txt") << "settings";
  FakeEsp32().set_fs_root(root);

  LITTLEFS.forceUnmount();
  Mount mount = LITTLEFS.mount();
  ASSERT_TRUE(mount.ok());
  std::unique_ptr<MultipassInputStream> file = mount.fopen("/config.txt");
  ASSERT_NE(nullptr, file);
  ASSERT_EQ(kOk, file->status());
  char contents[9] = {};
  EXPECT_EQ(std::size_t{8}, file->read(reinterpret_cast<byte*>(contents), 8));
  EXPECT_STREQ("settings", contents);
  file.reset();
  mount.close();
  LITTLEFS.forceUnmount();
  FakeEsp32().set_fs_root(previous);
  std::filesystem::remove_all(root);
}

}  // namespace
}  // namespace roo_io
