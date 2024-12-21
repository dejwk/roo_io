# roo_io

Common I/O classes and routines: data encoding/decoding, file and stream abstraction, byte iterators.

This library, with help from platform-specific add-ons (roo_io_espressif, roo_io_arduino, roo_io_sdfat) provides a unified filesystem abstraction over a variety of implementations, making it easy to write portable code for microcontrollers. The abstraction is somewhat similar to the Arduino filesystem abstraction, but with several significant improvements, such as:

* much more detailed error reporting,
* support for dynamic mounting (automatic mounting/unmounting),
* support for read-only mounts,
* support for graceful shutdown policies (lame-duck read-only mode, etc.)

Additional features:

* high-performance data encoders/decoders, supporting variety of types (in addition to the 'usual suspects', also e.g. variable-length integers, Base64, UTF-8),
* high-performance I/O abstractions (streams, iterators, etc.) over memory (RAM or Flash),
* extra utilities (e.g. formatted printing to string).
