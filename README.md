# roo_io

Common I/O classes and routines: data encoding/decoding, file and stream abstraction, byte iterators.

This library provides a unified filesystem abstraction over a variety of implementations, making it easy to write portable code for microcontrollers. Supported backend filesystems include: Arduino SD, ESP32 SDSPI (Arduino and esp-idf variants), ESP32 SDMMC (Arduino and esp-idf variants), and ESP32 SPIFFS (works both with Arduino and esp-idf).

The abstraction is somewhat similar to the Arduino filesystem, but with several significant improvements, such as:

* much more detailed error reporting,
* support for dynamic mounting (automatic mounting/unmounting),
* support for read-only mounts,
* support for graceful shutdown policies (lame-duck read-only mode, etc.)

Additional features:

* high-performance data encoders/decoders, supporting variety of types (in addition to the 'usual suspects', also e.g. variable-length integers, Base64, UTF-8, Cobs),
* high-performance I/O abstractions (streams, iterators, etc.) over memory (RAM or Flash),
* stream adapters for UART,
* extra utilities (e.g. formatted printing to string).
