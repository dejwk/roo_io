#pragma once

#if defined(ARDUINO) && defined(ARDUINO_ARCH_RP2040) && \
    (!defined(ROO_TESTING) || defined(ROO_IO_TEST_PICO_SDIO))

#include <FS.h>

#include <memory>

#include "roo_io/fs/filesystem.h"

namespace roo_io {

/// Mounts an SD card through Arduino-Pico's native four-bit SDIO transport.
/// Serialize access to the filesystem and its SDIO pins externally.
class PicoSdSdioFs final : public Filesystem {
 public:
  /// Uses @p clk and @p cmd for SD signals, with DAT0 through DAT3 on four
  /// consecutive GPIOs starting at @p dat0. @p clock_divider controls the
  /// SDIO PIO clock divider used for data transfers.
  PicoSdSdioFs(uint8_t clk, uint8_t cmd, uint8_t dat0, float clock_divider);

  /// Releases cached mounts and the SDIO transport.
  ~PicoSdSdioFs() override;

  /// Each instance owns one SDIO backend and cannot be copied.
  PicoSdSdioFs(const PicoSdSdioFs&) = delete;
  PicoSdSdioFs& operator=(const PicoSdSdioFs&) = delete;

  /// Probes the card without mounting. An active mount is checked with CMD13.
  MediaPresence checkMediaPresence() override;

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override;

 private:
  class SdioFilesystem;

  std::shared_ptr<SdioFilesystem> impl_;
  fs::FS fs_;
};

}  // namespace roo_io

#endif  // Arduino-Pico native SDIO
