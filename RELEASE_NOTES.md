# roo_io 2.3.0

- Added a unified I2C API for Arduino and native ESP-IDF, supporting 7-bit device addressing, reads, writes, and repeated-start transactions.
- Added ESP-IDF bus ownership management, existing-bus support, and configurable device timeouts.
- Added I2C documentation, Bazel targets, and ESP-IDF unit and host-emulation tests.
- Updated dependencies to `roo_backport` 1.2.4, `roo_logging` 1.5.10, and `roo_threads` 1.2.8, including PlatformIO minimum versions.
- Updated Bazel and CI dependencies to `rules_cc` 0.2.25 and `roo_testing` 2.1.2; added GoogleTest 1.18.0.bcr.1.
- Added consolidated release history.

---

# [roo_io 2.2.7](https://github.com/dejwk/roo_io/releases/tag/2.2.7)

Published 2026-08-29.

* Fixed a resource leak in POSIX directory handling: Directory now closes its underlying handle when destroyed, including after a directory read reaches end-of-stream.

---

# [roo_io 2.2.6](https://github.com/dejwk/roo_io/releases/tag/2.2.6)

Published 2026-08-29.

his release strengthens filesystem behavior and greatly expands ESP32 host-emulation support.

### Highlights

- Added runnable host-emulated examples for Arduino Serial, SD, and SDMMC, plus ESP-IDF UART, SD, and SDMMC build coverage.
- Migrated host builds to `roo_testing` 2.x profiles, with explicit Arduino and ESP-IDF configurations.
- Improved SD-over-SPI emulation: media presence and mounting now reflect whether the corresponding host directory exists.
- Fixed POSIX directory iteration to consistently omit `.` and `..`.
- Fixed a leaked POSIX directory handle: `Directory` now closes its underlying handle when destroyed, including after end-of-directory.
- Made ESP32 UART blocking reads use the API’s explicit `uint32_t` timeout type.
- Centralized AddressSanitizer configuration, resolved test races, and modernized CI automation.
- Updated supporting dependencies, including `roo_testing`, `roo_backport`, `roo_logging`, and `roo_threads`.

### Host-test usage

Host emulation now supports separate Arduino and ESP-IDF profiles:

```sh
bazel test ...
bazel test --config=roo_testing_arduino_esp32 ...
bazel test --config=roo_testing_idf_esp32 ...
```

No public source-level API migration is required.

---

# [roo_io 2.2.5](https://github.com/dejwk/roo_io/releases/tag/2.2.5)

Published 2026-08-15.

Split Bazel Targets by Platform Backend.

This makes it easier to avoid compilation issues in emulated builds.


---

# [roo_io 2.2.4](https://github.com/dejwk/roo_io/releases/tag/2.2.4)

Published 2026-08-07.

Re-formatted the source code for consistency.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.2.3...2.2.4

---

# [roo_io 2.2.3](https://github.com/dejwk/roo_io/releases/tag/2.2.3)

Published 2026-06-04.

* Fixed a couple of bugs in multipass stream iterators.
* Added a complete doxygen documentation.
* Added  a programming guide.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.2.2...2.2.3

---

# [roo_io 2.2.2](https://github.com/dejwk/roo_io/releases/tag/2.2.2)

Published 2026-05-16.

Fixed ASAN tests using the POSIX filesystem adapter and memory iterable.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.2.1...2.2.2

---

# [roo_io 2.2.1](https://github.com/dejwk/roo_io/releases/tag/2.2.1)

Published 2026-05-16.

## What's Changed
* fix for "missing initializer" error by @Gamadril in https://github.com/dejwk/roo_io/pull/3
* fixed unusable MemoryResource.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.2.0...2.2.1

---

# [roo_io 2.2.0](https://github.com/dejwk/roo_io/releases/tag/2.2.0)

Published 2026-04-14.

This release brings fast card presence detection for ESP32, for SDFS (Arduino), SD-SPI (Arduino and esp-idf), and SD-MMC (Arduino and esp-idf). With that, you can quickly detect when the card was inserted or removed, without mounting or unmounting it.

Presence detection takes:
* ~260 microseconds for SD-SPI, 
* 3.5ms for SD-MMC when absent; <200 microseconds when present.

Note that, in comparison, mounting the card can take anywhere from couple hundred ms to over 1500 ms, so our presence detection is two-three orders of magnitude faster.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.1.8...2.2.0

---

# [roo_io 2.1.8](https://github.com/dejwk/roo_io/releases/tag/2.1.8)

Published 2026-03-29.

* Added new memory utilities to check if the memory region is filled with a repeated pattern of length 1, 2, 3, or 4 bytes.
* Fix in PatternFill routines, to accept larger fill areas. (Before, it would overflow on 32KB).

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.1.7...2.1.8

---

# [roo_io 2.1.7](https://github.com/dejwk/roo_io/releases/tag/2.1.7)

Published 2026-03-25.

Bugfix in ringpipe (off-by-one).

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.1.6...2.1.7

---

# [roo_io 2.1.6](https://github.com/dejwk/roo_io/releases/tag/2.1.6)

Published 2026-02-26.

Bugfix: memory leak on destruction in output_stream_writer.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.1.5...2.1.6

---

# [roo_io 2.1.5](https://github.com/dejwk/roo_io/releases/tag/2.1.5)

Published 2026-02-25.

* Added support for loading/storing/reading/writing floating point values (on IEEE754-compliant systems)
* Fixed compiled warnings. Builds clean now!
* Added doxygen-style documentation.
* Updated dependencies.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.1.4...2.1.5

---

# [roo_io 2.1.4](https://github.com/dejwk/roo_io/releases/tag/2.1.4)

Published 2026-01-30.

Compilation fix in one of the files used by roo_display with esp-idf.

---

# [roo_io 2.1.3](https://github.com/dejwk/roo_io/releases/tag/2.1.3)

Published 2026-01-26.

## What's Changed
* espidf framework build fixes by @Gamadril in https://github.com/dejwk/roo_io/pull/2,
* fixes for espidf filesystem; added examples,
* fixed tests that got broken by the recent Bazel release.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.1.2...2.1.3

---

# [roo_io 2.1.2](https://github.com/dejwk/roo_io/releases/tag/2.1.2)

Published 2026-01-06.

Updated dependencies.

---

# [roo_io 2.1.1](https://github.com/dejwk/roo_io/releases/tag/2.1.1)

Published 2026-01-06.

Updated the roo_logging dependency.

---

# [roo_io 2.1.0](https://github.com/dejwk/roo_io/releases/tag/2.1.0)

Published 2026-01-06.

* Made compatible with esp-idf, and Raspberry Pi Pico (RP2040) SMP.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.0.3...2.1.0

---

# [roo_io 2.0.3](https://github.com/dejwk/roo_io/releases/tag/2.0.3)

Published 2025-11-12.

Added support for LittleFS mounts on ESP32.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.0.2...2.0.3

---

# [roo_io 2.0.2](https://github.com/dejwk/roo_io/releases/tag/2.0.2)

Published 2025-10-31.

* Refreshed dependencies;
* Better CI;
* added .gitignore.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.0.1...2.0.2

---

# [roo_io 2.0.1](https://github.com/dejwk/roo_io/releases/tag/2.0.1)

Published 2025-10-14.

Fixing the circular dependency on roo_io_arduino which has crept into the latest release.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/2.0.0...2.0.1

---

# [roo_io 2.0.0](https://github.com/dejwk/roo_io/releases/tag/2.0.0)

Published 2025-10-14.

New features:
* Added many filesystem implementations, based on Arduino SD, as well as ESP32-specific (SPIFFS, SDSPI, and SDMMC, the last two in Arduino and esp-idf variants)
* Arduino-specific stream adapters moved over from the roo_io_arduino library (so that this one is now self-contained)
* Added stream adapters for UART, both for generic Arduino, and optimized for ESP32,
* Added a simple ring-buffer implementation,
* Added some usage examples.

Refactorings:
* Buffered iterators simplified, likely to have smaller overall memory impact.

Bug fixes:
* a bug fixed in buffered output stream.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/1.4.0...2.0.0

---

# [roo_io 1.4.0](https://github.com/dejwk/roo_io/releases/tag/1.4.0)

Published 2025-10-05.

* Added a RingPipe class, implementing in-memory buffer for blocking producer/consumers.
* Added address sanitizer tests.
* Bug fix: virtual destructor of the DirectoryImpl class, potentially causing to memory leaks.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/1.3.0...1.4.0

---

# [roo_io 1.3.0](https://github.com/dejwk/roo_io/releases/tag/1.3.0)

Published 2025-08-10.

Made roo_io a Bazel module, to enable use in unit tests of depending libraries.

Adding CI/CD (unit tests running on push).

**Full Changelog**: https://github.com/dejwk/roo_io/compare/1.2.0...1.3.0

---

# [roo_io 1.2.0](https://github.com/dejwk/roo_io/releases/tag/1.2.0)

Published 2025-07-04.

* Improved code comments.
* Allowing MacAddress to be used in hashtables.
* Fixed several compilation issues.

---

# [roo_io 1.1.0](https://github.com/dejwk/roo_io/releases/tag/1.1.0)

Published 2024-12-28.

Fix: the streams and directory objects now keep the mount alive until they are closed or destroyed.

**Full Changelog**: https://github.com/dejwk/roo_io/compare/1.0.2...1.1.0

---

# [roo_io 1.0.2](https://github.com/dejwk/roo_io/releases/tag/1.0.2)

Published 2024-12-22.

Minor bug fixes.

---

# [roo_io 1.0.1](https://github.com/dejwk/roo_io/releases/tag/1.0.1)

Published 2024-12-21.

Minor bug fixes and improvements.

---

# [roo_io 1.0.0](https://github.com/dejwk/roo_io/releases/tag/1.0.0)

Published 2024-12-21.

Initial release.

---

