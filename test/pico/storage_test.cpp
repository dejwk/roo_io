#include <FSImpl.h>
#include <LittleFS.h>
#include <SD.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "roo_io/fs/arduino/littlefs.h"
#include "roo_io/fs/arduino/sdfs.h"
#include "roo_testing/devices/storage/sd_spi/sd_spi.h"
#include "roo_testing/frameworks/arduino_pico_support/filesystem.h"
#include "roo_testing/microcontrollers/rp2040/fake_pico.h"

namespace {
struct SdWiring {
  SdWiring() {
    FakePico().attachSpiDevice(card, 14, 12, 15);
    FakePico().attachOutput(13, card.cs());
  }
  FakeSdSpiCard card;
} sd_wiring;

// Forwards to the real pruning fake, but makes rename reject existing targets
// so the shared mount must use its remove-and-rename replacement fallback.
class NonReplacingFsImpl : public fs::FSImpl {
 public:
  explicit NonReplacingFsImpl(fs::FSImplPtr backend)
      : backend_(std::move(backend)) {}

  bool setConfig(const fs::FSConfig& config) override {
    return backend_->setConfig(config);
  }

  bool begin() override { return backend_->begin(); }
  void end() override { backend_->end(); }
  bool format() override { return backend_->format(); }
  bool info(fs::FSInfo& info) override { return backend_->info(info); }

  fs::FileImplPtr open(const char* path, fs::OpenMode mode,
                       fs::AccessMode access) override {
    return backend_->open(path, mode, access);
  }

  bool exists(const char* path) override { return backend_->exists(path); }

  fs::DirImplPtr openDir(const char* path) override {
    return backend_->openDir(path);
  }

  bool rename(const char* from, const char* to) override {
    ++rename_calls;
    return !backend_->exists(to) && backend_->rename(from, to);
  }

  bool remove(const char* path) override {
    const bool success = backend_->remove(path);
    removed = removed || success;
    return success;
  }

  bool mkdir(const char* path) override {
    return !(fail_parent_creation && removed) && backend_->mkdir(path);
  }

  bool rmdir(const char* path) override {
    const bool success = backend_->rmdir(path);
    removed = removed || success;
    return success;
  }

  bool stat(const char* path, fs::FSStat* stat) override {
    return backend_->stat(path, stat);
  }

  int rename_calls = 0;
  bool removed = false;
  bool fail_parent_creation = false;

 private:
  fs::FSImplPtr backend_;
};

// Keeps the pruning fake's storage/lifetime while substituting its FS dispatch.
class NonReplacingFilesystem : public roo_testing::PicoFilesystem {
 public:
  NonReplacingFilesystem() : PicoFilesystem(true) {
    implementation = std::make_shared<NonReplacingFsImpl>(_impl);
    _impl = implementation;
  }

  std::shared_ptr<NonReplacingFsImpl> implementation;
};

// Verifies separate isolated native roots, the imported FS/File/Dir API,
// persistence across remount, and path confinement including symlink rejection.
TEST(PicoStorage, CoreFsApiAndIsolation) {
  roo_testing::PicoFilesystem a;
  roo_testing::PicoFilesystem b;
  ASSERT_NE(a.hostRoot(), b.hostRoot());
  ASSERT_TRUE(a.begin());
  ASSERT_TRUE(b.begin());
  ASSERT_TRUE(a.mkdir("/folder"));
  fs::File file = a.open("/folder/value", "w");
  ASSERT_TRUE(static_cast<bool>(file));
  EXPECT_EQ(file.write(reinterpret_cast<const uint8_t*>("abc"), 3), 3u);
  file.close();
  EXPECT_FALSE(b.exists("/folder/value"));
  a.end();
  ASSERT_TRUE(a.begin());
  file = a.open("/folder/value", "r");
  ASSERT_TRUE(static_cast<bool>(file));
  EXPECT_EQ(file.read(), 'a');
  EXPECT_TRUE(file.seek(2));
  EXPECT_EQ(file.read(), 'c');
  file.close();
  fs::Dir dir = a.openDir("/folder");
  ASSERT_TRUE(dir.next());
  EXPECT_STREQ(dir.fileName().c_str(), "/folder/value");
  EXPECT_FALSE(dir.next());
  EXPECT_FALSE(a.exists("/../anything"));
  ASSERT_EQ(symlink(b.hostRoot().c_str(), (a.hostRoot() + "/escape").c_str()),
            0);
  EXPECT_FALSE(static_cast<bool>(a.open("/escape/outside", "w")));
}

// Verifies the real Roo mount/file lifecycle on SD: bus configuration, update
// policies, directory enumeration, read-only mounts and file-held ownership.
TEST(PicoStorage, SdLifecycleAndConfiguration) {
  roo_io::SD.forceUnmount();
  SDFS.setMediaPresent(true);
  SDFS.setMountFailure(false);
  sd_wiring.card.setPresent(true);
  roo_io::SD.setUnmountingPolicy(roo_io::Filesystem::kUnmountEagerly);
  roo_io::SD.setSPI(SPI1);
  roo_io::SD.setCsPin(13);
  roo_io::SD.setFrequency(2000000);
  roo_io::Mount mount = roo_io::SD.mount();
  ASSERT_TRUE(mount.ok());
  EXPECT_EQ(::SD.spi(), &SPI1);
  EXPECT_EQ(::SD.frequency(), 2000000u);
  EXPECT_EQ(::SD.chipSelect(), 13);
  EXPECT_EQ(mount.mkdir("/docs"), roo_io::kOk);
  auto output = mount.fopenForWrite("/docs/file", roo_io::kTruncateIfExists);
  const roo_io::byte bytes[] = {roo_io::byte{1}, roo_io::byte{2}};
  ASSERT_EQ(output->writeFully(bytes, 2), 2u);
  output->close();
  auto append = mount.fopenForWrite("/docs/file", roo_io::kAppendIfExists);
  ASSERT_EQ(append->writeFully(bytes, 2), 2u);
  append->close();
  auto conflict = mount.fopenForWrite("/docs/file", roo_io::kFailIfExists);
  EXPECT_EQ(conflict->status(), roo_io::kFileExists);
  conflict->close();
  EXPECT_EQ(mount.stat("/docs/file").size(), 4u);
  EXPECT_EQ(mount.rename("/docs/file", "/docs/renamed"), roo_io::kOk);
  auto directory = mount.opendir("/docs");
  ASSERT_TRUE(directory.read());
  EXPECT_FALSE(directory.read());
  directory.close();
  auto input = mount.fopen("/docs/renamed");
  mount.close();
  EXPECT_TRUE(SDFS.mounted());
  roo_io::byte read[4];
  EXPECT_EQ(input->read(read, 0), 0u);
  EXPECT_EQ(input->status(), roo_io::kOk);
  ASSERT_EQ(input->readFully(read, 4), 4u);
  EXPECT_EQ(read[3], bytes[1]);
  input->close();
  EXPECT_FALSE(SDFS.mounted());
  mount = roo_io::SD.mount();
  directory = mount.opendir("/docs");
  mount.close();
  EXPECT_TRUE(SDFS.mounted());
  directory.close();
  EXPECT_FALSE(SDFS.mounted());
  roo_io::SD.setMountingPolicy(roo_io::Filesystem::kMountReadOnly);
  mount = roo_io::SD.mount();
  EXPECT_EQ(mount.remove("/docs/renamed"), roo_io::kReadOnlyFilesystem);
  mount.close();
  roo_io::SD.setMountingPolicy(roo_io::Filesystem::kMountReadWrite);
  sd_wiring.card.setPresent(false);
  EXPECT_EQ(roo_io::SD.mount().status(), roo_io::kNoMedia);
  sd_wiring.card.setPresent(true);
  EXPECT_EQ(roo_io::SD.checkMediaPresence(), roo_io::Filesystem::kMediaPresent);
  SDFS.setMountFailure(true);
  EXPECT_EQ(roo_io::SD.mount().status(), roo_io::kGenericMountError);
  SDFS.setMountFailure(false);
}

// Verifies LittleFS uses the same public storage contract, mount errors are
// observable, forceUnmount invalidates handles and media loss fails I/O.
TEST(PicoStorage, LittleFsErrorsAndUnmount) {
  roo_testing::PicoFilesystem backend;
  roo_io::ArduinoLittleFs flash(backend);
  flash.setUnmountingPolicy(roo_io::Filesystem::kUnmountEagerly);
  backend.setMountFailure(true);
  EXPECT_EQ(flash.mount().status(), roo_io::kGenericMountError);
  backend.setMountFailure(false);
  roo_io::Mount mount = flash.mount();
  ASSERT_TRUE(mount.ok());
  auto output = mount.fopenForWrite("/file", roo_io::kTruncateIfExists);
  const roo_io::byte byte{7};
  ASSERT_EQ(output->writeFully(&byte, 1), 1u);
  output->close();
  flash.forceUnmount();
  EXPECT_EQ(mount.status(), roo_io::kNotMounted);
  mount.close();
  mount = flash.mount();
  auto input = mount.fopen("/file");
  backend.setMediaPresent(false);
  roo_io::byte read;
  EXPECT_EQ(input->read(&read, 1), 0u);
  EXPECT_EQ(input->status(), roo_io::kReadError);
  input->close();
  mount.close();
}
// Verifies obsolete force-unmounted handles cannot tear down a fresh mount,
// and destroying a wrapper releases its cached lazy mount while still derived.
TEST(PicoStorage, StaleHandlesAndCachedUnmount) {
  roo_testing::PicoFilesystem backend;
  {
    roo_io::ArduinoLittleFs flash(backend);
    auto old = flash.mount();
    ASSERT_TRUE(old.ok());
    flash.forceUnmount();
    auto fresh = flash.mount();
    ASSERT_TRUE(fresh.ok());
    old.close();
    EXPECT_TRUE(backend.mounted());
    auto output = fresh.fopenForWrite("/fresh", roo_io::kTruncateIfExists);
    ASSERT_EQ(output->status(), roo_io::kOk);
    fresh.close();
    EXPECT_TRUE(backend.mounted());
    output->close();
    EXPECT_FALSE(backend.mounted());
  }
  {
    roo_io::ArduinoLittleFs cached(backend);
    cached.setUnmountingPolicy(roo_io::Filesystem::kUnmountLazily);
    auto mount = cached.mount();
    ASSERT_TRUE(mount.ok());
    mount.close();
    EXPECT_TRUE(backend.mounted());
  }
  EXPECT_FALSE(backend.mounted());
}

// Verifies Roo removal preserves each ancestor even when the native LittleFS
// API prunes them. Explicit rmdir removes only its requested empty directory.
TEST(PicoStorage, LittleFsPreservesParentDirectories) {
  roo_testing::PicoFilesystem backend(true);
  roo_io::ArduinoLittleFs flash(backend);
  auto mount = flash.mount();
  ASSERT_TRUE(mount.ok());
  ASSERT_EQ(mount.mkdir("/parent"), roo_io::kOk);
  ASSERT_EQ(mount.mkdir("/parent/child"), roo_io::kOk);
  auto file = mount.fopenForWrite("/parent/child/file", roo_io::kFailIfExists);
  ASSERT_EQ(file->status(), roo_io::kOk);
  file->close();
  ASSERT_EQ(mount.remove("/parent/child/file"), roo_io::kOk);
  EXPECT_TRUE(mount.stat("/parent").isDirectory());
  EXPECT_TRUE(mount.stat("/parent/child").isDirectory());
  ASSERT_EQ(mount.rmdir("/parent/child"), roo_io::kOk);
  EXPECT_TRUE(mount.stat("/parent").isDirectory());
  EXPECT_EQ(mount.rmdir("/parent"), roo_io::kOk);
}

// Verifies base-class rename replacement dispatches to the Pico deletion hook
// for both files and directories, restoring destination parents before moving.
TEST(PicoStorage, LittleFsRenameReplacementPreservesParents) {
  for (bool directory : {false, true}) {
    NonReplacingFilesystem backend;
    roo_io::ArduinoLittleFs flash(backend);
    roo_io::Mount mount = flash.mount();
    ASSERT_TRUE(mount.ok());
    ASSERT_EQ(mount.mkdir("/parent"), roo_io::kOk);
    ASSERT_EQ(mount.mkdir("/parent/child"), roo_io::kOk);
    if (directory) {
      ASSERT_EQ(mount.mkdir("/source"), roo_io::kOk);
      ASSERT_EQ(mount.mkdir("/parent/child/target"), roo_io::kOk);
    } else {
      std::unique_ptr<roo_io::OutputStream> source =
          mount.fopenForWrite("/source", roo_io::kFailIfExists);
      ASSERT_EQ(source->status(), roo_io::kOk);
      const roo_io::byte value{0x42};
      ASSERT_EQ(source->writeFully(&value, 1), 1u);
      source->close();
      std::unique_ptr<roo_io::OutputStream> target =
          mount.fopenForWrite("/parent/child/target", roo_io::kFailIfExists);
      ASSERT_EQ(target->status(), roo_io::kOk);
      target->close();
    }
    ASSERT_EQ(mount.rename("/source", "/parent/child/target"), roo_io::kOk);
    EXPECT_EQ(backend.implementation->rename_calls, 2);
    EXPECT_TRUE(mount.stat("/parent").isDirectory());
    EXPECT_TRUE(mount.stat("/parent/child").isDirectory());
    EXPECT_EQ(mount.stat("/source").status(), roo_io::kNotFound);
    EXPECT_EQ(mount.stat("/parent/child/target").isDirectory(), directory);
    if (!directory) {
      std::unique_ptr<roo_io::MultipassInputStream> input =
          mount.fopen("/parent/child/target");
      roo_io::byte value;
      ASSERT_EQ(input->readFully(&value, 1), 1u);
      EXPECT_EQ(value, roo_io::byte{0x42});
      input->close();
    }
  }
}

// Verifies failure to restore pruned parents reports an I/O error for file and
// directory deletion rather than reporting a fully successful Roo operation.
TEST(PicoStorage, LittleFsParentRestorationFailure) {
  for (bool directory : {false, true}) {
    NonReplacingFilesystem backend;
    roo_io::ArduinoLittleFs flash(backend);
    roo_io::Mount mount = flash.mount();
    ASSERT_TRUE(mount.ok());
    ASSERT_EQ(mount.mkdir("/parent"), roo_io::kOk);
    if (directory) {
      ASSERT_EQ(mount.mkdir("/parent/target"), roo_io::kOk);
    } else {
      std::unique_ptr<roo_io::OutputStream> file =
          mount.fopenForWrite("/parent/target", roo_io::kFailIfExists);
      ASSERT_EQ(file->status(), roo_io::kOk);
      file->close();
    }
    backend.implementation->fail_parent_creation = true;
    EXPECT_EQ(directory ? mount.rmdir("/parent/target")
                        : mount.remove("/parent/target"),
              roo_io::kUnknownIOError);
    EXPECT_EQ(mount.stat("/parent/target").status(), roo_io::kNotFound);
    EXPECT_EQ(mount.stat("/parent").status(), roo_io::kNotFound);
  }
}

}  // namespace
