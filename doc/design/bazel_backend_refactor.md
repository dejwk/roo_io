# Design: Split Bazel Targets by Platform Backend

## Status

Proposed.

## Problem

The current `:roo_io` Bazel target builds almost every source file through a
single `src/**/*.cpp` glob. Platform selection is made by C++ preprocessor
macros such as `ESP32`, `ARDUINO`, `ESP_PLATFORM`, and `ROO_TESTING`, which
must be supplied while dependencies themselves compile.

Consequently, a host test that enables the Arduino ESP32 configuration builds
all Arduino ESP32 backends, including ones its device does not use. In
particular, the Makerfabs display uses the SPI-SD backend
(`roo_io::ArduinoSdSpiFs` / `roo_io::SD_SPI`), but the same target also builds
the Arduino SD-MMC backend (`ArduinoSdMmcFs` / `roo_io::SD_MMC`).

This couples unrelated backends and makes their SDK dependencies implicit. The
SD-MMC issue exposed the problem:

- `arduino/sdmmc.cpp` uses the Arduino `::SD_MMC` object but `:roo_io` does
  not declare the `SD_MMC` library as a dependency.
- The file includes `sdmmc_cmd.h`, although it does not use any symbol from
  that header. The only host-test copy is an internal ESP-IDF simulator stub,
  not a public Bazel dependency.

## Goals

- Compile only the storage backends a consumer selects.
- Make each backend's SDK and link dependencies explicit.
- Keep `roo_io` usable by host tests and embedded Bazel consumers without
  project-local source exclusions.
- Preserve existing C++ APIs such as `roo_io::SD_SPI` and `roo_io::SD_MMC`.
- Avoid making an internal `sdmmc_cmd.h` simulator stub part of the public
  roo_testing API.

## Non-goals

- Replacing the Arduino build system or changing the public filesystem API.
- Emulating physical SD-MMC card behavior beyond what roo_testing already
  supports.
- Removing platform preprocessor guards from public headers in this change.

## Immediate Correctness Fix

Before or alongside the target split:

1. Remove the unused `#include "sdmmc_cmd.h"` from
   `src/roo_io/fs/esp32/arduino/sdmmc.cpp`.
2. Add the direct dependency below to the Bazel target that compiles that
   source:

   ```starlark
   "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/SD_MMC",
   ```

`SD_MMC` owns `SD_MMC.h` and the global `::SD_MMC` implementation used by the
Arduino backend. The ESP-IDF-only SD-MMC probe remains guarded out of
`ROO_TESTING` builds, so a host-test `sdmmc_cmd` shim is neither needed nor the
right abstraction.

This small change should allow downstream users to remove any local
source-exclusion patch immediately.

## Proposed Bazel Target Layout

Replace the broad source glob with explicit source groups. The exact names may
change, but the ownership boundaries should remain.

```text
:roo_io                         platform-neutral core and common APIs
:arduino_fs                     generic Arduino filesystem adapters
:esp32_arduino_sdspi            Arduino SPI-SD backend
:esp32_arduino_sdmmc            Arduino SD-MMC backend
:esp32_idf_sdspi                ESP-IDF SPI-SD backend
:esp32_idf_sdmmc                ESP-IDF SD-MMC backend
:esp32_littlefs                 ESP32 LittleFS backend
:esp32_spiffs                   ESP32 SPIFFS backend
```

`:roo_io` should contain only sources that do not require an optional Arduino
or ESP-IDF storage backend. It remains the basic dependency for common stream,
filesystem, POSIX, data, and memory APIs. A consumer that instantiates a
platform backend must depend on that backend target directly.

For example:

```starlark
cc_library(
    name = "esp32_arduino_sdspi",
    srcs = [
        "src/roo_io/fs/esp32/arduino/sdspi.cpp",
        "src/roo_io/fs/esp32/arduino/internal/sd_spi_probe.cpp",
    ],
    deps = [
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/SD",
    ],
)

cc_library(
    name = "esp32_arduino_sdmmc",
    srcs = [
        "src/roo_io/fs/esp32/arduino/sdmmc.cpp",
        "src/roo_io/fs/esp32/internal/sd_mmc_probe.cpp",
    ],
    deps = [
        ":roo_io",
        "@roo_testing//roo_testing/frameworks/arduino-esp32-2.0.4/libraries/SD_MMC",
    ],
)
```

The SD-MMC probe source is compiled only where its `ESP_PLATFORM` production
guard enables code. Its real ESP-IDF dependencies belong on the ESP-IDF or
production-oriented backend target, rather than on the generic core.

## Consumer Migration

Consumers must name the backend they use. For example, the Makerfabs
ESP32 TFT device references `roo_io::SD_SPI`, so its Bazel target should add:

```starlark
"@roo_io//:esp32_arduino_sdspi",
```

A consumer using `roo_io::SD_MMC` instead adds
`@roo_io//:esp32_arduino_sdmmc`. This is intentionally explicit: a C++ header
reference should have a corresponding Bazel dependency that supplies the
implementation.

During migration, keep `:roo_io` as the platform-neutral public target. Do
not make it depend on every optional backend for backwards compatibility; that
would retain the problem this refactor removes. Release notes should call out
the new direct dependency requirement.

## roo_testing Responsibilities

`roo_testing` should continue to expose the Arduino `SD_MMC` library as the
public owner of `SD_MMC.h` and `::SD_MMC`. Its current dependencies on the
ESP-IDF SD-MMC driver, FATFS, and VFS are the right direction.

Do not expose `components/spi_flash/sim/stubs/sdmmc/include/sdmmc_cmd.h` as a
general dependency solely for roo_io. It is an internal, minimal simulator
header and the Arduino roo_io SD-MMC implementation does not use it. If a
future host-test backend genuinely calls SD-MMC command APIs, introduce a
purpose-built public shim target with defined behavior and tests.

## Validation Plan

1. Add build coverage for each backend target under the intended host-test
   configuration (`ARDUINO`, `ESP32`, and `ROO_TESTING`).
2. Add an SD-MMC test that instantiates `ArduinoSdMmcFs` and mounts the
   roo_testing filesystem, if the existing fake ESP32 VFS supports it.
3. Keep or add the equivalent SPI-SD test for `ArduinoSdSpiFs`.
4. Add a consumer-level analysis/build test for the Makerfabs display that
   depends on `:esp32_arduino_sdspi` and links `roo_io::SD_SPI`.
5. Run the complete roo_io test suite and build all backend targets after the
   split.

## Rollout

1. Land the immediate SD-MMC correctness fix and regression test.
2. Introduce the backend targets while retaining the core `:roo_io` target.
3. Update first-party consumers, especially roo_display product targets, to
   depend on the backend they reference.
4. Release roo_io and roo_display together or document the compatible version
   pair.
5. Remove downstream workarounds that exclude `arduino/sdmmc.cpp`.
