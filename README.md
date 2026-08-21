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
