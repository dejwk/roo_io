# Pico I/O support

## Objective

Provide usable roo_io facilities on Pico SDK, Arduino-Pico bare-metal and
Arduino-Pico FreeRTOS while retaining ESP32 behavior.

## Motivation

The Arduino transport sources are largely portable, but several Bazel targets
still select ESP32 dependencies. SDK-only code cannot build through logging,
blocking pipes require unsupported bare-metal waits, and filesystem emulation
selects an ESP32 implementation solely because ROO_TESTING is defined.

## Background

Pico uses the pinned SDK and Arduino-Pico core in roo_testing. Runtime identity
is separate from frontend/SoC identity; the emulator supports one FreeRTOS core.
See [Pico integration](../../roo_testing/doc/pico_integration.md) and
[the FreeRTOS runtime](../../roo_testing/doc/pico_freertos.md).

## Requirements

Portable memory, data and text I/O must work without Arduino or ESP-IDF.
Supported transfers must preserve framing, release ownership on failures, and
obey stream progress/status contracts. Storage must retain mount lifetimes,
read-only policies and file update semantics. Existing ESP32 consumers remain
source compatible. Each feature must have focused tests and explicit runtime
compatibility; unsupported bare-metal blocking APIs must fail at the boundary.

## Design Overview

Use existing interfaces and concrete backends. Separate portable core, generic
filesystem, ringbuffer and threaded ringpipe build targets. Keep :roo_io as a
compatible aggregate of the facilities supported on the selected runtime.

Arduino backends continue to use Wire/Stream/SPI. Platform selects choose matching
framework dependencies. SDK transports borrow initialized SDK controllers and
expose the same device/stream contracts; multi-call SPI scopes retain CS and
serialize against other Roo devices on that controller under FreeRTOS.

Arduino-Pico filesystem support uses the core's FS APIs over SDFS and LittleFS.
Host filesystem objects implement that same API over isolated native directories.
Host paths model storage contents, not SD wire timing or flash wear. Card presence
and mount failures have explicit fake controls. Production Arduino SD supports
bus/frequency configuration and optional card-detect input; without card detect,
unmounted presence remains unknown rather than inventing a card probe.

## Design Details

### Backend identity and ownership

Physical Arduino-Pico selects FreeRTOS only when __FREERTOS is enabled; emulated
runtime identity retains precedence. A portable logging buffer supplies SDK-only
formatting without framework headers. Bus handles and streams borrow initialized
controllers; devices/scopes must finish before those controllers are released.

### Progress and failure

Zero-length I/O does nothing. writeFully loops over partial writes and reports
kWriteError when the underlying stream makes no progress. flush delegates to the
underlying Stream. Reads retain the existing blocking-until-data contract and
yield between unsuccessful attempts. Arduino-Pico Wire uses a shared TX/RX
buffer without cross-call locking; callers serialize complete I2C conversations.
Unread RAM bytes do not reserve the hardware controller for a task.

### Supported storage

The first storage release covers Arduino-Pico SD and LittleFS. Native SDK
filesystem drivers are a separate integration. Mounts own their backend lifetime;
existing file objects retain their mount. Hardware qualification uses compile
smoke tests plus a board sketch where hardware is available; host tests do not
claim electrical SD or flash validation.

## Proposed API

Existing :roo_io, :i2c and :spi names remain. Add focused :core, :filesystem,
:ringbuffer, :ringpipe and :pico_uart targets. Public bus headers select Pico SDK
when neither Arduino nor IDF is active and Pico identity is present. Arduino
storage exposes configured SPI/frequency and an optional borrowed card-detect
predicate. LittleFS gets a concrete Filesystem wrapper.

## Implementation Plan

Authoring: [C++ guidance](../.github/instructions/general-cpp-code-authoring-instructions.md).

1. **Dependencies** (roo_logging, roo_threads): portable log buffer, correct
   physical Pico runtime selection. Commit: `Pico I/O stage 1: Make dependencies
   portable.` Validate logging and selection under SDK/Arduino/IDF.
2. **Core** (roo_io): focused libraries and independent test fixtures. Commit:
   `Pico I/O stage 2: Separate portable and threaded facilities.` Validate generic
   tests on Pico and reject blocking pipes on bare-metal.
3. **Arduino routing** (roo_io, roo_testing fixtures): Pico framework selects and
   real fake-device tests. Commit: `Pico I/O stage 3: Connect Arduino adapters to
   Pico.` Validate Wire/SPI/UART on both Arduino runtimes.
4. **Contracts** (roo_io): I2C failure cleanup and Stream progress/flush fixes.
   Commit: `Pico I/O stage 4: Restore adapter progress and cleanup contracts.`
   Validate partial/zero writes, reads, flush and bus reuse after failure.
5. **SDK transports** (roo_io, roo_testing): SDK I2C/SPI/UART adapters and fake
   integration. Commit: `Pico I/O stage 5: Add native SDK transports.` Validate
   byte/error/framing parity and runtime serialization.
6. **Storage APIs** (roo_io): SD configuration/card detection and LittleFS
   wrapper. Commit: `Pico I/O stage 6: Complete Arduino-Pico storage APIs.`
   Validate firmware compilation, lifecycle and configuration behavior.
7. **Storage emulation** (roo_testing, roo_io): Pico FS/SDFS/LittleFS adapters
   with isolated roots and injected failures. Commit: `Pico I/O stage 7: Emulate
   Pico filesystems.` Validate the public storage lifecycle in both runtimes.
8. **Release integration** (affected libraries and registry metadata): canonical
   profiles, CI, coherent dependency pins and consumer regressions. Commit:
   `Pico I/O stage 8: Validate and align the profile matrix.` Validate normal and
   ASAN configurations serially, using persistent Bazel storage and global limits.

Each stage can use separate commits per library or independently testable slice.

## Testing Plan

Use real public adapters over existing fake buses/devices. Generic tests have no
filesystem dependency. Exercise both Arduino time modes and SDK-only code;
FreeRTOS tests verify another task progresses after contention/error cleanup.
Retain ESP32 Arduino/IDF regressions and use roo_time, roo_rtc and roo_display as
consumers. Build firmware smoke tests with exceptions/RTTI disabled where the
core requires it. Record hardware/toolchain availability separately from host
results. Source imports keep their recorded upstream ownership and patches.

## Caveats

Single-core host scheduling does not establish SMP/IRQ/DMA or electrical timing
correctness. SDK and Arduino device setup remains the application's responsibility.
Blocking pipes require a threaded runtime. Native descriptor/flash behavior is
outside fake storage's behavioral contract.

### Rejected Alternatives

#### Route Pico through ESP32 host identity

This would choose incompatible APIs and invalidate profile isolation. Pico keeps
its actual frontend and SDK identity.

#### Rewrite existing consumer interfaces

The existing device, stream and filesystem contracts already express the needed
operations. Concrete backends and focused build targets minimize migration.

## Future Work

SDK-native storage drivers, DMA/asynchronous I/O, multicore and IRQ integration
need separate contracts and qualification.
