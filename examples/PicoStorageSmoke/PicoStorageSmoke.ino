#include <LittleFS.h>
#include <roo_io/fs/arduino/littlefs.h>

#include <cstring>

#ifndef ROO_IO_PICO_SMOKE_SD
#define ROO_IO_PICO_SMOKE_SD 1
#endif
#ifndef ROO_IO_PICO_SMOKE_FORMAT_IF_NEEDED
#define ROO_IO_PICO_SMOKE_FORMAT_IF_NEEDED 0
#endif
#if ROO_IO_PICO_SMOKE_SD
#include <SPI.h>
#include <roo_io/fs/arduino/sdfs.h>
#endif

namespace {
constexpr const char* kDirectory = "/roo-io-smoke";
constexpr const char* kFile = "/roo-io-smoke/data.bin";
constexpr const char* kRenamed = "/roo-io-smoke/renamed.bin";

// Records each board-level contract without terminating later cleanup checks.
bool Check(bool passed, const char* operation) {
  Serial.printf("%s %s\n", passed ? "PASS" : "FAIL", operation);
  return passed;
}

// Verifies physical flash I/O through Roo's public filesystem and stream APIs.
// The caller provisions the partition or explicitly enables formatting.
bool TestFiles() {
  roo_io::ArduinoLittleFs flash;
  roo_io::Mount mount = flash.mount();
#if ROO_IO_PICO_SMOKE_FORMAT_IF_NEEDED
  if (!mount.ok()) {
    Serial.println("Explicit test option: formatting unmounted LittleFS");
    if (!Check(LittleFS.format(), "format")) return false;
    mount = flash.mount();
  }
#endif
  if (!Check(mount.ok(), "mount")) return false;
  // Never replace a pre-existing user directory, including interrupted tests.
  if (!Check(!mount.exists(kDirectory), "test directory available"))
    return false;
  bool passed = Check(mount.mkdir(kDirectory) == roo_io::kOk, "mkdir");
  roo_io::byte pattern[64];
  for (size_t i = 0; i < sizeof(pattern); ++i) {
    pattern[i] = static_cast<roo_io::byte>((i * 3 + 0x31) & 0xff);
  }
  {
    auto output = mount.fopenForWrite(kFile, roo_io::kFailIfExists);
    for (int i = 0; i < 4; ++i) {
      passed =
          Check(output->writeFully(pattern, sizeof(pattern)) == sizeof(pattern),
                "write block") &&
          passed;
    }
    output->flush();
    passed = Check(output->status() == roo_io::kOk, "flush") && passed;
    output->close();
  }
  {
    auto output = mount.fopenForWrite(kFile, roo_io::kAppendIfExists);
    passed = Check(output->writeFully(pattern, 16) == 16, "append") && passed;
    output->close();
  }
  passed = Check(mount.stat(kFile).size() == 272, "stat size") && passed;
  {
    auto conflict = mount.fopenForWrite(kFile, roo_io::kFailIfExists);
    passed = Check(conflict->status() == roo_io::kFileExists,
                   "existing file rejected") &&
             passed;
    conflict->close();
  }
  passed =
      Check(mount.rename(kFile, kRenamed) == roo_io::kOk, "rename") && passed;
  {
    auto directory = mount.opendir(kDirectory);
    const bool read = directory.read();
    passed = Check(read, "directory entry") && passed;
    if (read) {
      passed = Check(std::strcmp(directory.entry().name(), "renamed.bin") == 0,
                     "directory basename") &&
               passed;
    }
    passed =
        Check(!directory.read() && directory.status() == roo_io::kEndOfStream,
              "directory end") &&
        passed;
    directory.close();
  }
  // Drop every mount/file before remounting so reads cannot use an open file's
  // cache.
  mount.close();
  passed = Check(!flash.isMounted(), "eager unmount") && passed;
  mount = flash.mount();
  passed = Check(mount.ok(), "remount") && passed;
  {
    auto input = mount.fopen(kRenamed);
    roo_io::byte received[64]{};
    for (int i = 0; i < 4; ++i) {
      const size_t count = input->readFully(received, sizeof(received));
      passed = Check(count == sizeof(received) &&
                         std::memcmp(pattern, received, sizeof(received)) == 0,
                     "flash block readback") &&
               passed;
    }
    passed = Check(input->readFully(received, 16) == 16 &&
                       std::memcmp(pattern, received, 16) == 0,
                   "appended readback") &&
             passed;
    input->seek(64);
    passed = Check(input->status() == roo_io::kOk &&
                       input->readFully(received, sizeof(received)) ==
                           sizeof(received) &&
                       std::memcmp(pattern, received, sizeof(received)) == 0,
                   "seek readback") &&
             passed;
    passed =
        Check(input->read(received, 0) == 0 && input->status() == roo_io::kOk,
              "zero read preserves status") &&
        passed;
    mount.close();
    passed = Check(flash.isMounted(), "file retains mount") && passed;
    input->close();
    passed = Check(!flash.isMounted(), "file close releases mount") && passed;
  }
  flash.setMountingPolicy(roo_io::Filesystem::kMountReadOnly);
  mount = flash.mount();
  passed = Check(mount.remove(kRenamed) == roo_io::kReadOnlyFilesystem,
                 "read-only mount") &&
           passed;
  mount.close();
  flash.setMountingPolicy(roo_io::Filesystem::kMountReadWrite);
  auto stale = flash.mount();
  flash.forceUnmount();
  mount = flash.mount();
  stale.close();
  passed = Check(mount.ok() && mount.stat(kRenamed).size() == 272,
                 "stale handle preserves fresh mount") &&
           passed;
  passed = Check(mount.remove(kRenamed) == roo_io::kOk, "remove") && passed;
  passed = Check(mount.rmdir(kDirectory) == roo_io::kOk, "rmdir") && passed;
  mount.close();
  return passed;
}

// Verifies lazy-cache destruction unmounts while its concrete wrapper is alive.
bool TestCachedUnmount() {
  {
    roo_io::ArduinoLittleFs flash;
    flash.setUnmountingPolicy(roo_io::Filesystem::kUnmountLazily);
    auto mount = flash.mount();
    if (!Check(mount.ok(), "lazy mount")) return false;
    mount.close();
    if (!Check(flash.isMounted(), "lazy mount retained")) return false;
  }
  FSInfo info;
  return Check(!LittleFS.info(info), "lazy wrapper destruction unmounts");
}

// A serial request runs one finite, repeatable hardware qualification pass.
void RunSmoke() {
#if defined(__FREERTOS)
  Serial.println("ROO_IO_STORAGE_SMOKE runtime=freertos");
#else
  Serial.println("ROO_IO_STORAGE_SMOKE runtime=bare-metal");
#endif
  LittleFS.setConfig(LittleFSConfig(false));
#if ROO_IO_PICO_SMOKE_SD
  roo_io::SD.setSPI(SPI);
  roo_io::SD.setCsPin(17);
  roo_io::SD.setFrequency(4000000);
  auto sd = roo_io::SD.mount();
  Serial.printf("SD status=%d\n", sd.status());
#else
  Serial.println("SKIP SD: no module configured");
#endif
  const bool files = TestFiles();
  const bool cached = TestCachedUnmount();
  Serial.printf("ROO_IO_STORAGE_RESULT=%s\n",
                files && cached ? "PASS" : "FAIL");
  Serial.flush();
}
}  // namespace

void setup() { Serial.begin(115200); }

void loop() {
  static bool started = false;
  if (!started && static_cast<bool>(Serial)) {
    started = true;
    RunSmoke();
  }
  if (Serial.available() > 0 && Serial.read() == 't') RunSmoke();
  delay(50);
}
