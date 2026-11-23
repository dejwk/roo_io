#pragma once

#include <FSImpl.h>

enum {
  SD_CARD_ERROR_ACMD6 = 1,
  SD_CARD_ERROR_ACMD13,
  SD_CARD_ERROR_ACMD51,
  SD_CARD_ERROR_READ_CRC,
  SD_CARD_ERROR_READ_TIMEOUT,
  SD_CARD_ERROR_CMD8,
};

constexpr uint32_t CARD_STATUS_ERROR = UINT32_MAX;

struct SdioConfig {
  SdioConfig(uint8_t clk, uint8_t cmd, uint8_t dat0, float divider) {}
};

struct SdioState {
  int init_error = 0;
  bool volume_ok = true;
  bool responds = true;
  int card_begins = 0;
  int volume_begins = 0;
  int status_queries = 0;
  int ends = 0;
};

extern SdioState sdio_state;

class FsDateTime {
 public:
  static void setCallback(void (*callback)(uint16_t*, uint16_t*)) {}
};

class SdFat {
 public:
  bool cardBegin(SdioConfig config) {
    ++sdio_state.card_begins;
    return sdio_state.init_error == 0;
  }

  bool volumeBegin() {
    ++sdio_state.volume_begins;
    return sdio_state.volume_ok;
  }

  int sdErrorCode() const { return sdio_state.init_error; }
  SdFat* card() { return this; }
  bool syncDevice() { return true; }

  uint32_t status() {
    ++sdio_state.status_queries;
    return sdio_state.responds ? 0 : CARD_STATUS_ERROR;
  }

  void end() { ++sdio_state.ends; }
};

namespace sdfs {

class SDFSImpl : public fs::FSImpl {
 public:
  bool setConfig(const fs::FSConfig&) override { return false; }
  bool begin() override { return false; }
  void end() override { _mounted = false; }
  bool format() override { return false; }
  bool info(fs::FSInfo&) override { return false; }
  bool exists(const char*) override { return false; }
  bool rename(const char*, const char*) override { return false; }
  bool remove(const char*) override { return false; }
  bool mkdir(const char*) override { return false; }
  bool rmdir(const char*) override { return false; }
  bool stat(const char*, fs::FSStat*) override { return false; }
  fs::DirImplPtr openDir(const char*) override { return nullptr; }
  fs::FileImplPtr open(const char*, fs::OpenMode, fs::AccessMode) override {
    return nullptr;
  }

 protected:
  static void dateTimeCB(uint16_t*, uint16_t*) {}
  SdFat* getFs() { return &fs_; }
  bool _mounted = false;

 private:
  SdFat fs_;
};

}  // namespace sdfs
