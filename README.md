# roo_io

## ESP-IDF emulator coverage

The ESP32 UART adapter has an IDF-only host test and a runnable `app_main`
example. The SD-over-SPI and SDMMC sources also have compile-only example
targets. Their complete ESP-IDF hardware-driver link surface is not currently
shimmed by roo_testing, but API drift is still caught. All of these targets
deliberately reject the Arduino profile:

```sh
bazel test --config=roo_testing_idf_esp32 //test/espidf:uart_stream_test
bazel build --config=roo_testing_idf_esp32 \
  //examples/espidf/UART:uart \
  //examples/espidf/SD:sd \
  //examples/espidf/SDMMC:sdmmc
```

Common I/O classes and routines: data encoding/decoding, file and stream abstraction, byte iterators.

This library provides a unified filesystem abstraction over a variety of implementations, making it easy to write portable code for microcontrollers. Supported backend filesystems include: Arduino SD, ESP32 SDSPI (Arduino and esp-idf variants), ESP32 SDMMC (Arduino and esp-idf variants), ESP32 SPIFFS (works both with Arduino and esp-idf), as well as LittleFS.

The abstraction is somewhat similar to the Arduino filesystem, but with several significant improvements, such as:

* much more detailed error reporting,
* support for dynamic mounting (automatic mounting/unmounting),
* support for read-only mounts,
* support for fast card presence detection,
* support for graceful shutdown policies (lame-duck read-only mode, etc.)

Additional features:

* high-performance data encoders/decoders, supporting variety of types (in addition to the 'usual suspects', also e.g. variable-length integers, Base64, UTF-8, Cobs),
* high-performance I/O abstractions (streams, iterators, etc.) over memory (RAM or Flash),
* stream adapters for UART,
* extra utilities (e.g. formatted printing to string).

For a guided walkthrough of the filesystem, stream, iterator, and typed I/O
layers, see [the programming guide](doc/programming_guide.md).

## Host emulation

Host builds support both Arduino and ESP-IDF through roo_testing 2.0. With
Bazelisk 1.21 or newer, a plain command defaults to Arduino and prints a notice:

    bazel test ...
    bazel test ... --config=asan
    bazel test ... --config=roo_testing_arduino_esp32
    bazel test ... --config=roo_testing_idf_esp32
    .roo_testing/bin/test_all_profiles ...

The files under .roo_testing are vendored from roo_testing; follow their
canonical-source headers when refreshing them.

Arduino examples are native runnable targets in their source packages. For
example:

    bazel run //examples/Serial:Serial

## I2C transactions

Include `roo_io/i2c/i2c.h` for `I2cMasterBusHandle` and `I2cSlaveDevice`.
The Arduino implementation lives in `i2c/arduino/`; the native ESP-IDF
implementation lives in `i2c/esp32/`. Both have separate `.h` and `.cpp` files.
Bazel users depend on `//:i2c`, independently of filesystem and stream adapters.

Initialize the bus in the application, construct a device with its unshifted
7-bit address, and call `device.init()` before transferring. `transmit` reports
complete success, `receive` returns a byte count, and `transmitReceive` performs
a combined write/read with repeated START and final STOP, succeeding only when
all bytes transfer. Buffers are borrowed only for the duration of the call.

On Arduino the bus borrows a `TwoWire` instance (default `Wire`). Initialize it
using the core's API; the ESP32 wrapper also offers `bus.init(sda, scl, hz)`.
Configure transfer timeouts on Wire using the core's supported API. Device
initialization does not initialize or take ownership of Wire. Serialize access
between tasks unless the Arduino core guarantees transaction synchronization.

On native ESP-IDF, `bus.init(sda, scl, hz)` creates an owned bus. Copies and
devices share its lifetime, and the last reference releases it. Alternatively,
wrap an existing `i2c_master_bus_handle_t` without allocating shared state. These
native-handle views and their copies never own the bus and cannot initialize a
new bus. Selecting an initialized bus by port also borrows the bus, but retains
shared state because port-based wrappers can instead become owners via `init()`.
Borrowed buses must outlive the devices. Each device owns its
registration and cannot be copied. Initialization returns false on failure and
can be retried. Device initialization is idempotent. Bus initialization must
precede device initialization and cannot replace an already resolved bus.
Serialize initialization/destruction externally. Native transfers default to a
1000 ms timeout; a nonnegative timeout can be supplied as the third device
constructor argument. Transfers never abort the application on an IDF error.

The native backend requires the modern IDF `driver/i2c_master.h` API and
`esp_driver_i2c` component, including support for `i2c_master_get_bus_handle`.
Arduino takes precedence when both `ARDUINO` and `ESP_PLATFORM` are defined.

Run native lifecycle and failure tests with:

```sh
bazel test --config=roo_testing_idf_esp32 //test/i2c:all
```

Tests cover the pin-routed RTC emulator as well as scripted initialization and
transfer failures. Arduino RTC integration and Wire failure tests live in
`roo_time_ds3231`. A temporary `roo_testing` local override supplies the modern
IDF emulator until its release; update its path when using a different checkout.
