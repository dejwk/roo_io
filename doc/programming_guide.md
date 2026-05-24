# Programming guide

## Before you start

### Purpose

Use `roo_io` when your program needs to move bytes between storage, memory,
and device I/O without rewriting the same low-level logic for each target.
Typical examples include reading configuration files, scanning directories,
parsing binary records, writing logs, adapting UART or `Stream` objects to a
common API, or reusing the same file-handling code across multiple boards.

The library solves two recurring embedded problems.

First, it gives you a portable filesystem layer. The same application code can
often work with SD cards, flash filesystems, Arduino, and ESP-IDF targets,
with most target-specific differences confined to setup.

Second, it gives you reusable byte-oriented I/O building blocks. Files,
memory buffers, and stream-like devices can all be handled through a common
model, so parsing and encoding code does not have to be tightly coupled to one
specific source or sink. Board-specific differences are confined to setup.
The same application logic can then mount, enumerate, open,
parse, and update files through one API.

Semantically, the filesystem API in `roo_io` aims to combine the flexibility
of POSIX with the simplicity of Arduino. It is C++ first, focuses on embedded
applications, and strives to feel familiar to Arduino programmers, but gives
you more control when needed. For example:
* When `mkdir()` or another filesystem function fails, you can distinguish
  between benign causes such as a directory already existing, application
  errors such as an invalid path, or system errors such as the filesystem not
  being mounted or a write fault.
* You can quickly (in milliseconds) check if an SD card is inserted in the slot,
  without mounting it. Mounting can be controlled via convenient policies.


## Part 1: Getting started

### First portable filesystem example

`Filesystem`, `Mount`, and `Directory` are the main entry points for storage
code. The helper below lists the root directory. It works unchanged with
different `roo_io` backends once the backend-specific setup is in place.

```cpp
#include "roo_io.h"

using namespace roo_io;

void listRoot(Filesystem& fs) {
  Mount mount = fs.mount();
  if (!mount.ok()) {
    LOG(ERROR) << "Mount failed: " << mount.status();
    return;
  }

  Directory dir = mount.opendir("/");
  while (dir.read()) {
    LOG(INFO) << dir.entry().path();
  }
  if (dir.failed()) {
    LOG(ERROR) << "Directory read failed: " << dir.status();
  }

  // With the default unmounting policy, this gets automatically unmounted as
  // `mount` goes out of scope.
}
```

On an Arduino sketch using the standard SD stack, the setup might look like
this:

```cpp
#include "SPI.h"
#include "roo_io/fs/arduino/sdfs.h"

using namespace roo_io;

void setup() {
  SPI.begin(12, 13, 11);
  SD.setCsPin(10);
  listRoot(SD);
}

void loop() {}
```

On ESP-IDF SDSPI, the storage-specific setup changes, but the file logic does
not:

```cpp
#include "driver/spi_master.h"
#include "roo_io/fs/esp32/esp-idf/sdspi.h"

using namespace roo_io;

extern "C" void app_main() {
  spi_bus_config_t buscfg = {
      .mosi_io_num = 11,
      .miso_io_num = 13,
      .sclk_io_num = 12,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 0,
  };
  CHECK_EQ(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO), ESP_OK);

  SDSPI.setCsPin(10);
  listRoot(SDSPI);
}
```

Backend-specific setup stays near hardware initialization. The application code
above it can keep using the same `Mount`, `Directory`, and file APIs.

Mounting is explicit. `Filesystem::mount()` returns a `Mount` object
representing one mounted session. When that object goes out of scope, the
filesystem may unmount automatically depending on the unmounting policy.

Once you have a healthy mount, operations such as `stat()`, `opendir()`,
`remove()`, `rename()`, `mkdir()`, `rmdir()`, `fopen()`, and
`fopenForWrite()` act as embedded-friendly C++ counterparts of familiar
filesystem calls.

### Reading and writing typed binary data

Many embedded applications need to read and write structured binary data
without hand-encoding every field. `InputStreamReader` and
`OutputStreamWriter` wrap a stream and provide typed helpers for fixed-width
integers, strings, variable-length integers, and buffered access.

The following example writes a simple binary config record to a file and reads
it back later.

```cpp
#include "roo_io.h"
#include "roo_io/data/input_stream_reader.h"
#include "roo_io/data/output_stream_writer.h"

using namespace roo_io;

void writeConfig(Mount& mount, uint16_t version, roo::string_view name) {
  std::unique_ptr<OutputStream> file =
      mount.fopenForWrite("/config.bin", kTruncateIfExists);
  if (file->status() != kOk) {
    LOG(ERROR) << "Open for write failed: " << file->status();
    return;
  }

  OutputStreamWriter out(std::move(file));
  out.writeBeU32(0x524F4F49);  // "ROOI"
  out.writeBeU16(version);
  out.writeString(name);
  out.flush();

  if (out.status() != kOk) {
    LOG(ERROR) << "Write failed: " << out.status();
  }
}

bool readConfig(Mount& mount) {
  std::unique_ptr<MultipassInputStream> file = mount.fopen("/config.bin");
  if (file->status() != kOk) {
    LOG(ERROR) << "Open for read failed: " << file->status();
    return false;
  }

  InputStreamReader in(std::move(file));
  uint32_t magic = in.readBeU32();
  uint16_t version = in.readBeU16();
  std::string name = in.readString(32);

  if (in.status() != kOk && in.status() != kEndOfStream) {
    LOG(ERROR) << "Read failed: " << in.status();
    return false;
  }

  LOG(INFO) << "magic=" << magic << ", version=" << version
            << ", name=" << name;
  return true;
}
```

`InputStreamReader` and `OutputStreamWriter` are a good default in application
code. They keep parsing and encoding independent from whether the underlying
stream comes from a file, memory buffer, UART adapter, or something custom.

When opening an output file, `FileUpdatePolicy` lets you say what should happen
when the target already exists:

- `kFailIfExists`: treat existing file as an error,
- `kTruncateIfExists`: overwrite from the beginning,
- `kAppendIfExists`: append instead.


### Working with memory as I/O

Many embedded tasks start with bytes already in RAM or flash: network packets,
ROM tables, binary assets, captured test fixtures, or messages from another
subsystem.

Use a memory iterator when you want the tightest possible byte-walking code:

```cpp
#include "roo_io/data/read.h"
#include "roo_io/memory/memory_input_iterator.h"

using namespace roo_io;

void parsePacket(const roo::byte* begin, const roo::byte* end) {
  MemoryIterator in(begin, end);

  uint32_t message_id = ReadBeU32(in);
  uint64_t sequence = ReadVarU64(in);

  if (in.status() != kOk && in.status() != kEndOfStream) {
    LOG(ERROR) << "Packet parse failed: " << in.status();
    return;
  }

  LOG(INFO) << "message_id=" << message_id << ", sequence=" << sequence;
}
```

Use a memory stream when you want to reuse the same reader-based code that you
would use for files or UART:

```cpp
#include "roo_io/data/input_stream_reader.h"
#include "roo_io/memory/memory_input_stream.h"

using namespace roo_io;

void parsePacketAsStream(const roo::byte* begin, const roo::byte* end) {
  MemoryInputStream<const roo::byte*> stream(begin, end);
  InputStreamReader in(stream);

  uint32_t message_id = in.readBeU32();
  uint64_t sequence = in.readVarU64();

  if (in.status() != kOk && in.status() != kEndOfStream) {
    LOG(ERROR) << "Packet parse failed: " << in.status();
  }
}
```

Use an iterator when the buffer is already in hand and the cheapest byte-access
path matters. Use a stream when interface uniformity with the rest of the API
matters more.

> Streams vs iterators
>
> **Stream** is a common underlying abstraction for an open resource such as a
> file, UART, or an application-defined data producer. It is defined as a pure
> virtual contract. `InputStream` reads forward, `OutputStream` writes forward,
> and `MultipassInputStream` adds `size()`, `position()`, and `seek()`.
>
> **Iterator** is a template contract. Iterators are lightweight byte walkers
> that do not use virtual calls, and thus work well in low-level code when data
> needs to be read or written byte-by-byte from specific sources.

### Adapting Arduino streams and UART

`roo_io` also covers transient I/O. You can wrap Arduino and UART-style
interfaces and use the same typed reader and writer helpers as you would with
files.

At the generic Arduino level, `ArduinoStreamInputStream` and
`ArduinoStreamOutputStream` adapt an Arduino `Stream` object to the `roo_io`
stream interfaces.

For serial links, the library also provides UART-oriented adapters such as
`ArduinoSerialInputStream` and `ArduinoSerialOutputStream`, plus ESP32-specific
UART adapters when you want to work more directly with native UART hardware.

Here is a minimal example using an Arduino serial port as a typed output stream:

```cpp
#include "roo_io/data/output_stream_writer.h"
#include "roo_io/uart/arduino/serial_output_stream.h"

using namespace roo_io;

void setup() {
  Serial1.begin(115200);

  ArduinoSerialOutputStream serial_out(Serial1);
  OutputStreamWriter out(serial_out);

  out.writeBeU32(1);
  out.writeString("hello\n");
  out.flush();
}

void loop() {}
```

This is a good fit when you already own the framing or when the peer simply
expects a byte stream. If you need reliable framing, retransmission, explicit
message boundaries, or reconnect-aware behavior, use a dedicated transport
layer on top of the UART, such as `roo_transport`, rather than trying to re-create
those features manually.

### Example roadmap

Related examples:

- [SD](../examples/SD/SD.ino): Arduino SD setup, mounting, and directory
  traversal.
- [SDMMC](../examples/SDMMC/SDMMC.ino): Arduino SDMMC usage on supported ESP32
  targets.
- [Serial](../examples/Serial/Serial.ino): UART stream adapters and a simple
  framed serial exchange.
- [ESP-IDF SD](../examples/espidf/SD/main.cpp): ESP-IDF SDSPI setup.
- [ESP-IDF SDMMC](../examples/espidf/SDMMC/main.cpp): ESP-IDF SDMMC setup.

## Part 2: Features in depth

Part 1 showed the shortest path to useful code. Part 2 walks the library in
the order most application code encounters it: status handling first, then the
filesystem layer, then streams and typed readers or writers, then the
iterator-level codecs and memory helpers beneath them.

One practical note up front: `roo_io.h` is intentionally small. It brings in
the status and filesystem abstractions. Pull in additional headers from
`data/`, `memory/`, `text/`, `stream/`, or `uart/` as you start using those
layers.

### Status and error reporting

`roo_io` uses explicit `Status` values instead of exceptions or `errno`.
Filesystem operations surface those statuses through typed objects such as
`Mount`, `Directory`, and `Stat`. Streams and adapters expose `status()`
directly. In both styles, the important property is the same: you can tell not
only that something failed, but what kind of failure it was.

Two statuses are part of ordinary successful control flow:

- `kOk`: the object is usable and the last operation succeeded.
- `kEndOfStream`: a readable stream or iterator reached the end of available
  input.

Two more statuses often describe lifecycle state rather than an I/O fault:

- `kClosed`: the stream or handle is no longer open.
- `kNotMounted`: a mount handle is detached, invalidated, or mounting is
  currently disabled.

Everything else is a real failure. The point of the enum is that many of those
failures are actionable:

- `kDirectoryExists` may be harmless during initialization.
- `kInvalidPath` usually points to a caller bug.
- `kNoMedia` means there is no card or other removable media present.
- `kReadOnlyFilesystem` means the current mount policy or backend forbids
  writes.
- `kNoSpaceLeftOnDevice` means the backing store is full.

This is one of the biggest differences from Arduino-style boolean APIs. A
single `false` return collapses all failure modes together; `roo_io::Status`
lets application code distinguish absence of media, permissions, read-only
mode, directory-shape mistakes, and generic transport or storage faults.

### Filesystem lifecycle and mount policies

If your code starts from files or directories, `Filesystem`, `Mount`,
`Directory`, and `Stat` are the main types to learn.

A `Filesystem` represents a mountable resource such as an SD card slot, a
flash partition, or a POSIX-backed test filesystem. A `Mount` is a handle to a
mounted backend instance. `Directory` and `Stat` are operation results layered
on top of that mount.

`checkMediaPresence()` is intentionally separate from `mount()`. Use it when
you only need a quick card-present style signal. It does not mount the
filesystem and does not create mount state as a side effect.

`mount()` creates a new `Mount` handle. Multiple `Mount` objects can refer to
the same mounted backend. The first healthy handle causes the actual mount;
later ones reuse it. A failed mount handle reports one of a small set of mount
statuses, most commonly:

- `kNotMounted`, when mounting is disabled by policy,
- `kNoMedia`, when the backend can tell there is no backing media,
- `kGenericMountError`, when mounting fails for another reason.

`MountingPolicy` controls what future mounts are allowed to do:

- `kMountReadWrite`: allow normal read-write mounts.
- `kMountReadOnly`: create future mounts in read-only mode.
- `kMountDisabled`: refuse future mounts.

This policy affects only future mount handles. It does not retroactively change
ones that already exist.

`UnmountingPolicy` controls what happens when the last healthy `Mount` handle
goes away:

- `kUnmountEagerly`: automatically drop the backend mount.
- `kUnmountLazily`: keep the backend mount cached for reuse.

In normal application code, let this policy do the work. The usual style is to
create a `Mount`, use it, and let scope exit close it. `forceUnmount()` exists
for exceptional cases such as shutdown or backend reset; it invalidates all
live mount handles immediately, after which they report `kNotMounted`.

One detail worth remembering when lazy unmounting is enabled: `isMounted()`
and `isInUse()` are not the same thing. A filesystem can remain mounted for
reuse even after no user-visible `Mount` handles remain.

### Working with a mounted filesystem

Once you have a healthy `Mount`, the API shape is deliberately small.

Use `stat()` when you need existence, type, or file size. Use `exists()` as a
convenience when you only care about presence. Use `opendir()` to enumerate a
directory, `fopen()` to open a file for reading, and `fopenForWrite()` to open
or create one for writing.

Each of these calls returns either a typed result object or a typed stream in a
well-defined state. That is why `roo_io` code often reads like this:

1. Mount.
2. Attempt one operation.
3. Inspect the returned object's status.
4. Continue or branch on the specific failure.

`Directory` is worth one special note: `read()` returns `true` only when it has
produced a new entry. If it returns `false`, inspect `dir.status()` to tell
end-of-directory from error. `entry()` is only meaningful after a successful
`read()`.

### Stream layers, typed readers, and ownership

If your code already has an open file, memory range, serial link, or device
adapter, the next layer down is the stream layer.

The raw stream interfaces are byte-oriented:

- `InputStream` reads forward.
- `OutputStream` writes forward.
- `MultipassInputStream` is an `InputStream` that also supports `size()`,
  `position()`, and `seek()`.

They do not know about integers, strings, or record formats. They only move
bytes and report status.

The primitive contracts are:

- `read()` and `write()` may complete only part of the requested transfer.
- `tryRead()` and `tryWrite()` are the non-indefinitely-blocking variants.
- `readFully()` and `writeFully()` are helper loops layered on top of the
  primitive calls.
- `flush()` matters for output. A successful write may still be buffered until
  `flush()` succeeds.

Readable streams can report `kEndOfStream`. Output streams do not; they report
`kOk` or an error.

For structured binary data, the usual next step is not to work with raw
streams directly, but to wrap them:

- `InputStreamReader` for typed reads from any `InputStream`.
- `MultipassInputStreamReader` for typed reads plus `size()`, `position()`,
  and `seek()`.
- `OutputStreamWriter` for typed writes to any `OutputStream`.

These wrappers expose helpers for fixed-width integers, endianness, strings,
host-native blobs, variable-length integers, and IEEE754 floats or doubles
when enabled. They also use 64-byte internal buffers to avoid tiny upstream
operations.

Ownership is explicit:

- `InputStreamReader` and `OutputStreamWriter` can either borrow a stream by
  reference or take ownership via `std::unique_ptr`.
- `MultipassInputStreamReader` owns its `MultipassInputStream`.

After wrapping a stream, access it only through the wrapper. The wrapper keeps
buffer state of its own, so mixing direct raw-stream calls with buffered
wrapper calls produces incoherent state.

At the lower level, `BufferedInputStreamIterator`,
`BufferedMultipassInputStreamIterator`, and `BufferedOutputStreamIterator`
adapt stream objects to the iterator contract used by the codec helpers.

### Iterator-level codecs and direct memory helpers

Drop to this layer when you already have bytes in memory or when you need the
parsing or encoding logic to stay independent from any virtual stream type.

Most of the binary codec layer lives in free functions over the iterator
contract. That is why the same helpers work with memory iterators, buffered
stream iterators, and custom iterator types.

The core data helpers cover:

- fixed-width signed and unsigned integers,
- big-endian and little-endian layouts,
- 24-bit integer forms,
- variable-length unsigned integers,
- length-prefixed strings,
- IEEE754 float and double encoding when enabled.

This is the API exposed by headers such as `roo_io/data/read.h` and
`roo_io/data/write.h`. `InputStreamReader`, `MultipassInputStreamReader`, and
`OutputStreamWriter` are convenience layers built on top of those same
functions.

When the bytes are already contiguous in memory, there is an even lower-friction
option: the direct memory helpers in `roo_io/memory/load.h` and
`roo_io/memory/store.h`. Those functions load or store fixed-width values
directly from or to memory without setting up a stream or iterator object. They
are useful in packet parsers, ROM-table readers, or serializers that already
manage their own buffer pointers.

For contiguous input specifically, memory iterators also unlock a few helpers
that are intentionally tied to in-memory data, such as `ReadStringView()`,
which returns a non-owning view backed by the iterator's underlying buffer.

### Text helpers and bundled extras

The text layer is intentionally small.

For UTF-8, use helpers in `roo_io/text/unicode.h` such as:

- `Utf8Decoder` for incremental decoding,
- `DecodeUtfString()` or `DecodeUtfStringToVector()` for whole-string decode,
- `WriteUtf8Char()` for UTF-8 encoding.

For formatting or compact textual transforms, the main public helpers are:

- `Base64Encode()` in `roo_io/text/base64.h`,
- `StringPrintf()` and `StringVPrintf()` in `roo_io/text/string_printf.h`.

There are also bundled third-party components under `roo_io/third_party`, such
as the COBS implementation and UTF support internals. Those are useful when you
need them, but they are opt-in dependencies rather than the main public entry
points of the library.

### Backends and portability boundaries

The portable part of `roo_io` starts above backend setup. The non-portable part
is the code that binds the library to a particular board or runtime.

The code that mounts, enumerates, opens, parses, and updates files can usually
stay the same across targets. The code that initializes SPI, chooses SDSPI vs
SDMMC, selects a mount point, sets partition labels, or configures max-open
file counts is backend-specific.

That split is visible in the source tree:

- `fs/arduino/` holds Arduino-facing filesystem adapters.
- `fs/esp32/` holds ESP32-specific backends.
- `fs/posix/` provides host-side filesystems useful for tests and Linux tools.
- `stream/arduino/` and `uart/...` hold stream and UART adapters.

On ESP32 VFS-based filesystems, common configuration points include mount
point, bus configuration, max open files, read-only mode, and whether to
format on mount failure.

### Performance tradeoffs

`roo_io` favors small, composable abstractions. The biggest performance wins
usually come from choosing the right layer rather than micro-tuning one layer.

- Use `Filesystem`, `Mount`, `Directory`, and typed readers or writers in
  normal application code.
- Use iterators when the code is byte-hot and virtual-call overhead matters.
- Use memory helpers when the bytes are already contiguous and you do not need
  stream state.
- Avoid byte-at-a-time virtual calls on raw streams when a buffered reader,
  writer, or iterator would do.
- Use `checkMediaPresence()` when you only need a card-present style signal.
- Use lazy unmounting only when reducing mount churn is worth holding backend
  resources longer.

The default 64-byte buffers in the buffered stream iterators and typed reader
or writer wrappers are a reasonable compromise for many embedded targets. They
matter most when the surrounding code performs many tiny logical operations.

### Extension points and testing

`roo_io` is structured so that new sources, sinks, filesystems, and codecs do
not require special framework hooks.

To add a new byte source or sink, implement `InputStream`, `OutputStream`, or
`MultipassInputStream`, and follow the status plus partial-transfer contracts.

To add a new filesystem backend, derive from `Filesystem`, implement
`mountImpl()` and `unmountImpl()`, and return a `MountImpl` that translates the
backend's behavior into `roo_io::Status` values.

To add a new codec, start with free functions over the iterator contract. That
makes the codec usable with memory iterators, buffered stream iterators, and
custom iterator types. If it becomes a common pattern, add a typed reader or
writer wrapper later.

The test tree mirrors these layers: `test/core`, `test/data`, `test/text`,
`test/memory`, and `test/fs`. Filesystem behavior is exercised both with fake
filesystems and, where supported, with POSIX-backed paths. That is usually the
easiest way to validate filesystem semantics before moving to hardware-specific
integration testing.

## Part 3: Advanced topics

### Lifetime, ownership, and API contracts

The easiest `roo_io` mistakes are usually not algorithmic mistakes. They are
lifetime mistakes: wrapping the wrong object, assuming a borrowed wrapper will
leave the underlying stream open, or mixing buffered and unbuffered access to
the same source.

The first distinction to keep straight is borrowed versus owned wrappers.

- `InputStreamReader` can borrow an `InputStream&` or take ownership through a
  `std::unique_ptr<InputStream>`.
- `OutputStreamWriter` can do the same for `OutputStream`.
- `MultipassInputStreamReader` owns its `MultipassInputStream`.

Ownership affects deletion, but not close behavior. All three reader or writer
wrappers close the underlying stream on destruction or explicit `close()`. That
means a borrowed wrapper is still a lifecycle owner in the sense that it ends
the stream session. If you need the raw stream to outlive a temporary parsing
helper, keep the wrapper alive for the same scope or do not introduce the
wrapper in the first place.

The same general rule applies one layer down. `InputStream` and `OutputStream`
destructors call `close()`. For file-backed streams, that usually means closing
the actual file handle. For adapter types such as Arduino `Stream` wrappers or
ESP32 UART wrappers, `close()` typically closes the `roo_io` adapter state, not
the peripheral itself. In other words, closing a wrapper means "stop using this
adapter through `roo_io`", not necessarily "power down the device".

Buffered wrapper exclusivity is the second big rule. `InputStreamReader`,
`MultipassInputStreamReader`, `OutputStreamWriter`, and the buffered stream
iterators all keep internal buffer state. Once a stream is wrapped, do not read
or write the raw stream behind the wrapper's back. Doing so leaves the wrapper's
buffer and the underlying cursor out of sync.

At the filesystem layer, keep `Mount` semantics similarly explicit in your
head:

- Multiple `Mount` objects may share one backend mount.
- `Mount::close()` detaches only that handle.
- `Filesystem::forceUnmount()` invalidates all live mount handles.
- Under eager unmounting, dropping the last healthy `Mount` can invalidate the
  session automatically.

If a `Mount` may live across code that changes mount policy or may forcefully
unmount the backend, re-check `ok()` or `status()` before using it again.

Finally, remember that some result objects are intentionally transient.
`Directory::entry()` exposes a view into storage owned by the directory
iteration state. If you need the entry path or name after the next successful
`read()`, copy it.

### Performance and RAM tradeoffs

`roo_io` is designed for embedded systems, but that does not mean every layer
has the same cost. The main performance skill is to choose the cheapest layer
that still keeps the code clear.

The reader and writer wrappers buy simplicity and fewer tiny upstream I/O
operations, but they are not free. `InputStreamReader`,
`MultipassInputStreamReader`, and `OutputStreamWriter` each carry a 64-byte
buffer. That is usually a good trade for file and device I/O, but it is still
per-instance RAM. If you keep many wrappers alive at once, that buffering cost
becomes visible.

Memory iterators make the cost tradeoff more explicit:

- `UnsafeMemoryIterator` is the smallest option. It carries only a current
  pointer and never reports end-of-stream on its own.
- `MemoryIterator` adds bounds tracking and end-of-stream reporting.
- `MultipassMemoryIterator` adds position and seek support.

Use the safe variants by default when parsing untrusted or variable-length
input. Drop to the unsafe variant only when the bounds are already controlled
elsewhere and the smaller, cheaper iterator is actually useful.

The same "pay only for what you need" rule applies to stream choice:

- If the bytes are already contiguous in memory, direct helpers such as
  `LoadBeU32()` or `StoreLeU16()` are usually the lightest option.
- If you need structured parsing but no virtual dispatch, use iterators plus
  the free codec helpers.
- If you need interface uniformity across files, memory, and device I/O, use
  streams and typed readers or writers.

Filesystem policy affects cost too. Eager unmounting releases backend state as
soon as the last handle disappears. Lazy unmounting keeps the backend mounted
for reuse, which can reduce mount churn at the cost of holding resources
longer.

For in-process producer-consumer byte flow, `RingPipe` is the library's
fixed-capacity option. Its capacity is a real design parameter: too small and
producers or consumers block frequently; larger sizes reduce blocking at the
cost of RAM.

### Testing and debugging

The easiest way to debug `roo_io` code is to debug one layer at a time.

The test tree mirrors the library structure closely:

- `test/core` covers the buffered stream iterators and other core mechanics.
- `test/data` covers byte order helpers, free codecs, and typed readers or
  writers.
- `test/memory` covers iterators plus `load`, `store`, and related helpers.
- `test/text` covers utilities such as UTF-8 and formatted strings.
- `test/fs` covers filesystem semantics, including fake filesystem behavior and
  Arduino-facing filesystem adapters.
- `test/ringpipe` covers the in-memory producer-consumer pipe.

That organization is useful as a debugging guide too. If a bug shows up while
reading a file, first ask which layer is actually failing:

1. Is the mount or open operation failing?
2. Is the raw stream contract being violated?
3. Is the buffered wrapper being mixed with raw access?
4. Is the codec logic itself wrong?

For parser and encoder work, prefer host-side fixtures when possible. Memory
iterators, memory streams, and fake filesystems are easier to exercise
deterministically than live hardware. The more behavior you can validate there,
the less backend-specific debugging remains for later.

When debugging failures in application code, log the actual `Status` value
instead of collapsing it to success or failure. That is often enough to tell a
bad path from an exhausted stream, a missing card, or a write-after-close bug.

### Extending roo_io

Most real extensions fall into one of three categories.

#### New filesystem backend

Derive from `Filesystem`, implement `mountImpl()` and `unmountImpl()`, and
return a `MountImpl` subclass that translates backend behavior into
`roo_io::Status`. The important design work is not just opening files; it is
choosing a clean status mapping for `stat()`, directory iteration, file opens,
and write failures.

Treat fake or host-side validation as part of the backend work, not as later
cleanup. A backend is much easier to trust once you can exercise mount errors,
path-shape errors, read-only behavior, and directory traversal without real
hardware in the loop.

#### New `InputStream` or `OutputStream` adapter

Implement the smallest correct primitive set first:

- `read()` plus `status()` for input,
- `write()` plus `status()` for output,
- `close()` when the adapter has a meaningful closed state.

Override `tryRead()`, `tryWrite()`, `readFully()`, `writeFully()`, or `flush()`
only when the backend can do better than the default helper behavior.

Document close behavior clearly. Some adapters close an underlying resource;
others, like the Arduino `Stream` wrappers, only mark the adapter closed.

#### New codec or typed helper

Start with free functions over the iterator contract. That makes the new codec
usable with memory iterators, buffered stream iterators, and custom iterators
without committing to one transport or storage type.

Only add reader or writer member helpers after the free-function version has a
clear use case in repeated application code. That keeps the convenience layer
thin and prevents the library from growing too many one-off wrappers.

### Limits and non-goals

`roo_io` is intentionally low-level. It solves byte movement, typed binary I/O,
portable filesystem access, and adapter layering. It does not try to become a
full communication stack or runtime.

In particular, it is not:

- a reliable transport layer with framing, retransmission, or reconnect logic,
- a full async runtime or task scheduler,
- a schema-driven serialization framework,
- a database or transactional storage layer,
- a single umbrella include for every feature in the repository.

Those boundaries matter because they help keep the library composable. If you
need framed, reconnect-aware messaging over UART or sockets, build on top of
`roo_io` with a transport layer such as `roo_transport`. If you need structured
application schemas, use a higher-level codec on top of the byte primitives.

The portability boundary is also worth stating plainly: `roo_io` aims for
portable application logic, not identical backend cost or behavior. Different
filesystems and device adapters may differ in latency, blocking behavior, and
feature support even when the surface API is the same.

## Conclusion

The shortest way to choose an entry point is to start from the problem shape.

- If you need portable file or directory access, start with `Filesystem` and
  `Mount`.
- If you need typed binary I/O over an already-open stream, start with
  `InputStreamReader`, `MultipassInputStreamReader`, or
  `OutputStreamWriter`.
- If you need to parse or encode bytes already in RAM or flash, start with the
  memory helpers or iterator-level codecs.
- If you need to adapt `Serial`, Arduino `Stream`, or native UART APIs, use the
  stream and UART adapters.
- If you need reliable framing or transport semantics, move up a layer rather
  than trying to force `roo_io` itself to provide them.

Used that way, `roo_io` stays what it is best at: a compact substrate for
portable embedded I/O code that still lets you choose the right cost and
abstraction level for each call site.

