# Checked binary primitives and borrowed I/O adapters

## Objective

Move reusable binary encoding, validation and byte-accounting functionality from
roo_proto into roo_io, with explicit failure reporting and no mandatory heap
allocation. This document is an implementation handoff; it does not implement
the APIs it describes.

## Motivation

roo_proto currently validates a varint into a ten-byte temporary buffer and then
asks roo_io to decode that buffer. It also implements strict UTF-8 validation,
ZigZag conversion, byte limits and output counting. These operations are useful
to other binary formats and belong beside roo_io's existing primitives.

The immediate correctness issue is that the existing ReadVarU64 can shift past
the width of uint64_t when input has too many continuation bytes. Centralizing
checked decoding removes that risk for existing roo_io consumers as well.

## Background

- [Status](../../src/roo_io/status.h) is the common transport/filesystem result
  enum; [StatusAsString](../../src/roo_io/status.cpp) formats its values.
- [Input iterators](../../src/roo_io/core/input_iterator.h) expose read, skip and
  status, but **no status setter**. A scalar read's byte is meaningful only when
  the resulting status is kOk. End-of-stream is normally detected by a read
  beyond the available bytes, not by consuming the last byte successfully.
- [Output iterators](../../src/roo_io/core/output_iterator.h) support scalar and
  potentially short bulk writes. A bulk write reports the accepted byte count.
- [read.h](../../src/roo_io/data/read.h) and
  [write.h](../../src/roo_io/data/write.h) already implement unsigned varints,
  fixed-width values and byte-array transfers. A varint stores seven payload
  bits per byte and a continuation bit; uint64_t needs at most ten bytes.
- ZigZag maps signed integers to unsigned integers before varint encoding. It is
  different from casting a negative signed integer to uint64_t.
- [Memory input](../../src/roo_io/memory/memory_input_iterator.h) and
  [memory output](../../src/roo_io/memory/memory_output_iterator.h) iterators
  already provide bounded memory access.
- [InputStream](../../src/roo_io/core/input_stream.h) and
  [OutputStream](../../src/roo_io/core/output_stream.h) provide readFully and
  writeFully. Existing typed stream reader/writer objects have their own
  buffering and close semantics; this work does not change those semantics.
- [Unicode utilities](../../src/roo_io/text/unicode.h) decode and encode UTF-8,
  but their public decoder interface does not separately report malformed input.

The consumer motivating this extraction is the neighboring
[roo_proto design](../../../roo_proto/docs/design.md). Protobuf field numbers,
wire types, presence and schema rules remain outside roo_io.

## Requirements

1. Decode hostile binary input without invalid shifts or arithmetic overflow.
2. Report malformed encoding separately from missing bytes and transport errors.
3. Preserve successful decoding behavior, with explicit migration for the revised
   string APIs and their strict versus truncating policies.
4. Consume only the bytes needed for one value; preserve following records.
5. Offer byte-bounded reads and encoded-size counting without allocation.
6. Borrow streams and cursors without closing them or introducing read-ahead.
7. Keep protocol-specific limits and semantics in their owning libraries.
8. Include focused tests and public documentation with each implementation step.

## Design Overview

Keep **roo_io::Status unchanged**. Low-level checked decoders return **bool**;
the iterator's existing status distinguishes input failure from malformed data.
Stateful readers retain a separate sticky **data_error_** flag, expose
hasDataError(), and define ok() as status() == kOk && !hasDataError(). Their
status() continues to report input/transport status only. This lets callers read
several fields and check once without conflating transport and decoding errors.

Introduce three small reusable facilities:

- A checked varint codec and explicit ZigZag helpers in the data layer.
- A strict UTF-8 predicate in the text layer.
- Borrowed limited-input adapters and a counting output iterator in the core
  layer. A limited adapter owns only accounting/error state, while borrowing the
  underlying cursor. A counting iterator owns only a count, limit and status.

| Responsibility | Owner after extraction |
|---|---|
| Varint structure, overflow and integer transforms | roo_io |
| Strict UTF-8 byte-sequence validation | roo_io |
| Byte limits, stream borrowing and output counting | roo_io |
| Protobuf tags, groups, packed fields and unknown-field dispatch | roo_proto |
| Presence, required fields, nesting policy and field capacities | roo_proto |
| ParseDelimited/SerializeDelimited message convenience APIs | roo_proto |
| Bounded strings/repeated containers | roo_proto for now; separate roo_collections proposal |

A checked read looks like this:

```cpp
uint64_t length = 0;
if (!roo_io::ReadVarU64(input, length)) {
  // input.status() == kOk: malformed data; otherwise an input failure.
  return false;
}
// Validate the decoded length against this application's remaining bytes and
// resource budget before narrowing it or allocating storage.
```

## Design Details

### 1. Boolean checked decoders and sticky reader errors

Do not add kBadData or change StatusAsString. No separate DecodeStatus is needed.
The low-level overload is:

```cpp
template <typename InputIterator>
bool ReadVarU64(InputIterator& input, uint64_t& value);
```

Its contract does not imply transactional input rollback:

| Situation | Result | Output, consumption and iterator status |
|---|---|---|
| Complete representable varint | true | Assign value; consume through its terminator; status is kOk |
| Input status already non-kOk | false | Leave value/cursor unchanged; preserve that status |
| EOF before any byte or before a terminator | false | Leave value unchanged; consumed prefix is not rolled back; status is kEndOfStream |
| Underlying read reports a transport error | false | Leave value unchanged; preserve the exact error; do not inspect an invalid returned byte |
| Invalid tenth byte | false | Leave value unchanged; consume ten bytes, never an eleventh; transport status remains kOk |

For the tenth byte, only 0x00 and 0x01 are valid: both have their continuation
bit clear and no payload above bit 63. Validate before shifting. A tenth byte
with any other value is malformed even when no eleventh byte exists.

Accept non-minimal encodings whose values fit, such as 0x80 0x00 for zero and a
ten-byte encoding ending in 0x00. Canonical encoding is not a read requirement.
Do not drain the rest of a malformed continuation sequence; stream recovery
belongs to the caller. Input failure takes precedence over interpreting a byte
returned by that failing read.

Use a local accumulator and assign value only on success. The implementation
needs at most ten reads, one uint64_t accumulator and a small loop index; no
scratch byte array, allocation or rollback is needed. It never sets transport
status itself. After false, the caller checks input.status() to classify failure.

Keep the existing low-level `uint64_t ReadVarU64(input)` overload for source
compatibility, but mark it `[[deprecated("Use ReadVarU64(input, value)")]]`.
Implement it through the checked overload, returning zero for failure. It remains intentionally lossy: malformed input leaves status() == kOk.
Recommend the bool overload for direct low-level use with untrusted data. This
limitation does not apply to the stateful reader's value-returning method.

#### Stateful readers

InputStreamReader and MultipassInputStreamReader keep `uint64_t readVarU64()`.
Add a bool data_error_ member, initialized false, and these accessors:

```cpp
bool hasDataError() const { return data_error_; }

bool ok() const { return status() == kOk && !hasDataError(); }
```

On a failed checked decode, latch data_error_ only when the underlying iterator's
status is kOk. EOF and transport errors remain represented solely by that status.
The flag is monotonic until reset; a later successful-looking operation cannot
clear it. There is no need for a new public readVarU64(out) reader overload.

Representative implementation:

```cpp
uint64_t readVarU64() {
  if (data_error_) {
    return 0;
  }
  uint64_t value = 0;
  if (!ReadVarU64(in_, value)) {
    if (in_.status() == kOk) {
      data_error_ = true;
    }
  }
  return value;
}
```

**Fast path:** guard all other consuming methods, including raw read(), every
read*() convenience and skip(), with only `if (data_error_)`. Do not add an ok()
or transport-status precheck to each method: the underlying iterator already
honors sticky transport failures. Checked codecs still perform the status checks
necessary to avoid interpreting an invalid byte. Only the decode-failure path
needs a reader-level status query to classify the failure.

The readers already adapt their internal iterators. Add these direct guards;
do not introduce another iterator-adapter layer solely to hold decoding errors.
The new limited-input adapters in section 4 serve byte-window semantics, not
this error-state purpose.

Once data_error_ is true, consuming reader calls do not touch the iterator or
caller-provided output buffers. Early returns use safe initialized values such
as zero or an empty object, but no particular sentinel is part of the public
reader contract. After any error, returned values have **unspecified contents
but remain valid C++ values**; objects stay destructible and no undefined
behavior or uninitialized scalar return is permitted. Failed operations can
leave a valid partial result or partially written buffer; callers must check
ok() before using it. Preserve stronger existing guarantees, such as string
buffer termination on a first failed read where documented.

The unchanged-output-on-failure guarantee belongs to the low-level checked
output-argument overload. Do not impose it on every value-returning reader
convenience or make tests depend on an exact failure sentinel.

Construction and explicit reset clear data_error_. Move construction/assignment
transfer it along with input state, preserving existing moved-from object
guarantees. Closing a reader does not erase a detected data error. Cursor
movement alone does not clear the
flag. Reset does not promise stream resynchronization: after malformed data,
the caller must select a known record boundary before continuing. Preserve
existing ownership/close behavior. Account for the additional bool and ABI
padding when measuring reader sizes; introduce no heap allocation for the flag.

Document and demonstrate the intended usage:

```cpp
uint64_t id = reader.readVarU64();
uint32_t flags = reader.readLeU32();
uint64_t length = reader.readVarU64();
if (!reader.ok()) {
  // Discard the record. Inspect hasDataError() and status() for diagnosis.
  return false;
}
```

#### Checked string reads

Replace the low-level ReadCString, ReadString and ReadStringView APIs with
bool-returning functions and output arguments. All are strict by default;
ReadCString and ReadString also have explicitly named Truncated counterparts. All decode the length with checked
ReadVarU64. False with input status kOk denotes malformed encoding or a
representation/size-limit violation; false with non-kOk status denotes input
failure. These functions do not modify transport status to report data errors.
This intentionally replaces the old string signatures; migrate callers rather
than retaining ambiguous, silently truncating legacy overloads.

ReadCString requires capacity, including space for the trailing NUL, and a
size_t* len output argument. On success, len counts stored payload bytes,
excluding the terminator. Strict success requires encoded_length < capacity;
encoded_length == capacity already fails. Capacity zero returns false without
consuming input or touching buf, even for an empty encoded string: no terminated
result can fit. For positive capacity, buf must address that many writable
bytes. Terminate the buffer even on failure; len and other output contents on
failure are valid but unspecified. Output pointers must be non-null.

ReadCStringTruncated retains min(encoded_length, capacity - 1) bytes and writes
the terminator. Oversized fields succeed with len == capacity - 1 only after
the complete encoded payload has been consumed. Capacity zero follows the same
failure rule as the strict version; never compute capacity - 1 in that case.

ReadString takes std::string* output and optional max_size = SIZE_MAX, measured
in payload bytes. It replaces the output's contents, rather than appending.
Strict reads reject encoded lengths greater than max_size or the output
string's max_size() before reserving or consuming payload. Strongly recommend
an explicit max_size for untrusted or potentially erroneous input; the default
is not a resource budget. Allocation failure is not an encoding error and keeps
the project's existing allocation-failure behavior.

ReadStringTruncated retains min(encoded_length, max_size) bytes and consumes
the rest. Check the retained size against size_t and std::string::max_size()
before allocation. A zero max_size permits a successful empty result after
consuming the full field. Truncation bounds storage, not the time or I/O needed
to consume an attacker-controlled length; use a bounded input for that budget.

ReadStringView takes roo::string_view* output (the existing roo string-view
type) and optional max_size = SIZE_MAX as a validation limit. Reject encoded
lengths exceeding that limit or representable view size before consuming payload.
There is no truncated counterpart: a view does not copy or allocate its payload.
Retain the existing memory-iterator constraint and backing-memory lifetime
requirements. Views must never extend beyond available memory, including on
failure. Success returns the complete field and advances input past it. Do not
expose this method on buffered stream readers: buffer refill cannot preserve
the view's lifetime. A reader with stable memory backing can expose a
value-returning readStringView() with the same sticky-error rules.

Strict size/representation failures consume the length prefix but no payload;
malformed prefixes consume only bytes needed to detect the error. There is no
rollback. Truncated reads must verify consumption of the discarded suffix:
premature EOF or transport failure returns false even when the retained prefix
is complete. Use exact consumption consistent with the iterator contract, not
an unchecked skip. Keep wire-length accounting in uint64_t and chunk reads or
skips to size_t; never narrow an unrepresentable full length. Strict reads reject
unrepresentable output sizes; truncated reads can discard long suffixes without
allocating or representing the whole field in size_t.

String output arguments can be modified on failure; unlike checked scalar
outputs, they need not remain unchanged. Keep objects valid and buffers
terminated as specified above. These helpers operate on bytes: embedded NULs
are retained and counted, and neither variant validates UTF-8 or preserves code
point boundaries when truncating.

Both stream readers expose value-returning readCString/readCStringTruncated
(required capacity, returning size_t) and readString/readStringTruncated
(optional max_size, returning std::string). They use the bool helpers and latch
data_error_ on false with input status kOk, including strict size-limit failures.
Successful intentional truncation does not latch the flag. Preserve flag-only
entry guards: once latched, subsequent calls do not touch input or output
buffers. Returned values after failure are valid but unspecified.

### 2. ZigZag helpers

Add constexpr ZigZagEncode32/64 and ZigZagDecode32/64 in data/zigzag.h. Implement
the transforms with unsigned arithmetic and safe signed reconstruction, without
signed left-shift overflow or reliance on arithmetic right shift of negatives.

Add ReadZigZag32/64 in read.h and WriteZigZag32/64 in write.h, plus matching typed
reader/writer methods. Low-level reads return bool and leave their output
argument unchanged on failure. Reader methods return int32_t/int64_t values
and use the same sticky-error and unspecified-result contract as readVarU64().
Writes follow the existing void-returning primitive convention; callers inspect
the output iterator/writer status.

ReadZigZag32 first uses checked ReadVarU64, then rejects an encoded unsigned value
above UINT32_MAX by returning false with input status still kOk. ReadZigZag64
accepts the whole uint64_t range.
There is no ambiguous ReadVarS64 API. Protobuf signed int32/int64 sign extension,
32-bit truncation rules and enum handling stay in roo_proto. In particular, a
consumer requiring protobuf's uint32 truncation behavior reads uint64_t and
applies that protocol conversion rather than using a strict narrower helper.

### 3. Strict UTF-8 validation

Add `bool IsValidUtf8(roo::string_view text)` in text/unicode.h. It accepts empty
input, ASCII, embedded NUL and every valid Unicode scalar value encoded in
UTF-8. It rejects stray continuations, overlong encodings, incomplete sequences,
surrogates, codepoints above U+10FFFF and invalid leading bytes. Unicode
noncharacters remain valid scalar values; this is encoding validation, not text
normalization or application content policy.

The implementation is O(B) for B input bytes, O(1) auxiliary storage, and does
not allocate or modify input. Port roo_proto's original byte-level validator
with focused boundary tests. IsValidUtf8 does not call u8c::u8next_ or use
Utf8Decoder to detect errors. The existing [u8c implementation](../../src/roo_io/third_party/u8c.cpp)
substitutes U+FFFD on malformed input, but U+FFFD itself is a valid Unicode
scalar value: its UTF-8 encoding EF BF BD must pass validation. Rejecting a
decoded replacement character would therefore reject valid input. Validate the
original byte sequences directly, including sequence lengths and the restricted
leading/continuation-byte ranges. Keep u8c and Utf8Decoder's existing behavior
unchanged: validation and replacement-based decoding remain separate operations.
Include explicit regression cases accepting EF BF BD and rejecting malformed
input that u8c would also decode to U+FFFD.

### 4. Borrowed limited input

Provide `LimitedInputIterator<InputIterator>` in core/limited_input_iterator.h
and `LimitedInputStream` in core/limited_input_stream.h. The iterator form serves
inlined codecs over existing cursors; the stream form supports runtime nesting
and existing polymorphic transports. Share accounting logic where straightforward,
without adding a public type-erasure framework.

Each adapter borrows its source, keeps a remaining-byte count and a sticky local
Status, and exposes remaining(). Construction does not read. Stream close() marks
only the adapter kClosed; neither close() nor destruction closes the source.
The iterator has no ownership or close operation. Both adapters are non-copyable
and non-movable in this initial API so nested borrowed references remain stable.

Contracts:

- No delegated read requests more than remaining(). Successful transfers decrement
  it by the actual count, including a transferred prefix reported with an error.
- Reading exactly the last allowed byte succeeds. A later nonzero read reports
  local kEndOfStream without touching the source. Parent/source status is unchanged
  by reaching a child's boundary.
- Zero-length bulk reads are no-ops and do not turn an exhausted window into EOF.
- Bulk reads clamp to the local limit and preserve legitimate short transfers.
  The caller's existing exact-transfer helper can continue until satisfied/EOF.
- Premature source EOF also reports kEndOfStream. A nonzero remaining count lets
  a caller distinguish premature exhaustion from a consumed local window.
- Propagate source errors unchanged; data errors are not transport statuses.
  A synchronous read returning zero with kOk is a contract violation: latch
  kReadError to avoid
  pretending progress occurred or looping forever. LimitedInputStream::tryRead
  delegates nonblocking reads and permits zero/kOk without this conversion.
- skip consumes up to the boundary and reports local kEndOfStream when asked to
  cross it. Use bounded scratch space; never delegate an unbounded source skip.
- The iterator implements the complete documented iterator interface. The stream
  form integrates with inherited readFully; test short reads and error prefixes.
- An active child exclusively controls its borrowed cursor. Do not interleave
  direct parent/source reads while using a child. Destruction never drains unread
  bytes. Consume/skip the remainder explicitly before resuming at the next field.

Nest adapters around the same existing cursor; do not construct a new buffered
reader for every field. Wrapping an existing buffered iterator is valid: its
buffer can hold subsequent bytes, but they remain available through that same
iterator. A limited stream itself adds no read-ahead. Creating a buffered reader
inside a limited stream is a separate ownership/read-ahead choice and is not the
recommended field-window pattern.

A child window larger than its parent's remaining bytes cannot overread the
parent, but will eventually encounter premature EOF. roo_proto retains its early
length-versus-parent-boundary check for immediate diagnostics. Message byte/depth
budgets, group matching and protobuf error latching stay in roo_proto.

For N successful bytes, accounting is O(N) with O(1) state per adapter. Nesting
D wrappers can add O(D) accounting per delegated operation. No per-field buffer
or heap allocation is added. An iterator's state is roughly one pointer, one
size_t and one Status (12 bytes on common 32-bit ABIs, 24 on common 64-bit ABIs,
including alignment); a polymorphic stream wrapper also has a vtable pointer.
Measure sizeof on the tested ABI rather than promising these as universal sizes.

### 5. Counting output

Add CountingOutputSink in core/counting_output_sink.h. It implements
write(byte), write(const byte*, size_t), flush(), status(), and size(). Construct
it with an optional byte limit, default SIZE_MAX. It never dereferences bulk
write data, permitting a null pointer when only a length is available.

A successful write increments size and returns the full requested bulk count.
Use `requested > limit - size` before addition. Exceeding the limit (including
size_t overflow) latches kNoSpaceLeftOnDevice, leaves the count unchanged for
that call, and returns zero for a bulk write. Later writes are no-ops. Zero-byte
writes do not fail or change state; flush is a no-op. No separate overflow enum
is needed: this is a sink capacity failure, not malformed input.

Do not repurpose NullOutputStream: its current contract accepts no bytes and
reports a configured status. Counting output is a distinct sink. The new iterator
needs two size_t values and one Status and takes O(1) work per write, regardless
of the requested byte count. Serializer CPU work still occurs during a size pass.

### 6. Reuse and consumer boundary

Reuse roo_io memory iterators in place of roo_proto's duplicate private
MemoryIterator. Evaluate existing ReadByteArray/WriteByteArray and readFully/
writeFully at their actual call sites; preserve short-transfer and error-prefix
semantics rather than adding competing ReadExact/WriteExact APIs.

The migration must not route bounded roo_proto operations through an allocating
buffered stream reader: that would regress the existing zero-allocation path.
Keep a small borrowed protocol adapter where needed to map roo_io statuses and
provide the exact-transfer behavior expected by current primitive writers.
In particular, do not feed a short-writing stream adapter directly into the
existing WriteVarU64 bulk-write path without handling its short writes.

roo_proto keeps its Reader/Writer facade, tags, wire dispatch and protocol errors.
After a false checked decode, it maps input status kOk to kMalformed,
kEndOfStream inside an expected value to kTruncated, and other input failures
to its existing I/O outcome. When using a stateful roo_io reader, check ok() and
map hasDataError() to kMalformed instead of checking status() alone. Counting-sink
capacity failure maps to its capacity error. Generic adapters do not latch
protobuf callback failures or nesting limits in roo_io::Status.

## Proposed API

These are declaration sketches, not an instruction to publish incomplete APIs.
Each phase lands fully implemented, tested entry points.

```cpp
// Status remains unchanged.

template <typename InputIterator>
bool ReadVarU64(InputIterator& input, uint64_t& value);

template <typename InputIterator>
[[deprecated("Use ReadVarU64(input, value)")]]
uint64_t ReadVarU64(InputIterator& input);  // Legacy, intentionally lossy.

// String output pointers are required; capacity includes the NUL terminator.
template <typename InputIterator>
bool ReadCString(InputIterator& input, char* buf, size_t capacity, size_t* len);
template <typename InputIterator>
bool ReadCStringTruncated(InputIterator& input, char* buf, size_t capacity,
                          size_t* len);
template <typename InputIterator>
bool ReadString(InputIterator& input, std::string* output,
                size_t max_size = SIZE_MAX);
template <typename InputIterator>
bool ReadStringTruncated(InputIterator& input, std::string* output,
                         size_t max_size = SIZE_MAX);
// Same memory-iterator constraint as the existing ReadStringView.
template <typename MemoryInputIterator>
bool ReadStringView(MemoryInputIterator& input, roo::string_view* output,
                    size_t max_size = SIZE_MAX);

constexpr uint32_t ZigZagEncode32(int32_t value);
constexpr uint64_t ZigZagEncode64(int64_t value);
constexpr int32_t ZigZagDecode32(uint32_t value);
constexpr int64_t ZigZagDecode64(uint64_t value);

template <typename InputIterator>
bool ReadZigZag32(InputIterator& input, int32_t& value);
template <typename InputIterator>
bool ReadZigZag64(InputIterator& input, int64_t& value);
template <typename OutputIterator>
void WriteZigZag32(OutputIterator& output, int32_t value);
template <typename OutputIterator>
void WriteZigZag64(OutputIterator& output, int64_t value);

bool IsValidUtf8(roo::string_view text);

template <typename InputIterator>
class LimitedInputIterator;  // ctor(InputIterator&, size_t), remaining().
class LimitedInputStream;    // ctor(InputStream&, size_t), remaining().
class CountingOutputSink;  // explicit ctor(size_t limit = SIZE_MAX), size().
```

Typed readers retain readVarU64(), gain value-returning readZigZag32/64(),
and expose hasDataError()/ok(). Typed writers gain writeZigZag32/64(value).
Reader status() remains transport-only. Reset/move handle the added sticky flag
as specified in section 1; stream ownership and close behavior remain unchanged.

## Implementation Plan

Follow the repo-local [code-authoring instructions](../../.github/instructions/embedded-cpp-code-authoring.instructions.md).
Use Google C++ style, public Doxygen contracts, no exceptions/RTTI, focused
regression tests, and clang-format. Each numbered phase is one focused commit;
its commit body describes the concrete slice and cites this design.

### Phase 1: Checked unsigned varints and sticky readers

**Proposed commit:** `Checked binary primitives phase 1: add checked varint decoding.`

Implement the bool checked overload, deprecate and harden the legacy low-level
overload, and
add sticky data-error state plus hasDataError()/ok() to both reader classes.
Guard all consuming reader methods directly with data_error_; do not add
redundant transport prechecks or an error-only adapter. Leave Status and
StatusAsString unchanged. Document batch reads followed by a single ok() check,
the low-level versus reader failure contracts, and the status-only caller
migration in the programming guide in this same commit.

Validate zero, 127, 128, UINT64_MAX, every length boundary, accepted non-minimal
encodings, every truncated prefix, tenth-byte overflow/continuation, and an
invalid sequence followed by sentinel bytes. Verify the eleventh byte is never
consumed, output remains unchanged on failure, and exact transport errors and
preexisting statuses remain observable through the iterator. Test that malformed
bytes return false while the iterator's status stays kOk. For both readers,
verify the flag latches, transport failures do not spuriously latch it, and a
sequence of read*()/skip() calls after a data error causes zero iterator calls.
Use a counting test iterator/stream to distinguish no underlying I/O from no
consumption of already-buffered bytes. Cover reset, move and close state.
Verify that the legacy low-level overload emits a deprecation diagnostic while
the checked overload and reader.readVarU64() do not. Migrate internal callers to
the checked overload. Do not assert a particular reader return value after
failure. Review guards to
ensure they check only data_error_, not ok() or status().

Start with //test/data:read_test, //test/data:input_stream_reader_test and
//test/data:multipass_input_stream_reader_test. Add any focused test support
needed to observe reader cursor consumption without changing production APIs.

### Phase 2: Existing length-prefix callers

**Proposed commit:** `Checked binary primitives phase 2: harden string length prefixes.`

Replace the three low-level string-read signatures with bool/output-argument
APIs and add Truncated counterparts for ReadCString and ReadString. Require C-string capacity in both
low-level and reader APIs. Implement strict size checks, checked prefixes,
overflow-safe conversions and exact suffix consumption. Update value-returning
reader methods to latch data errors and add truncating counterparts. Preserve
the memory-only constraint for views. Migrate repository callers, examples and
documentation; use Truncated explicitly wherever truncation is intended.

Validate malformed prefixes, strict size rejection before payload consumption,
empty strings, capacities zero/one, payload lengths capacity - 1 and capacity,
embedded NULs, max_size zero, partially missing retained payload and discarded
suffixes, and UINT64_MAX lengths without oversized allocation. Verify truncated
success stops at the next field and output buffers remain terminated. Verify
views return the complete field, reject lengths exceeding max_size, and stay
within backing memory. Exercise narrowing on a 32-bit target when available;
otherwise use width-independent boundary tests and record the limitation.
Verify strict limit failures latch reader data_error_ with transport status kOk,
EOF does not latch it, successful truncation does not latch it, and later
consuming calls are suppressed after data errors. Do not assert exact failure
results beyond the validity/termination guarantees. Run read_test and affected
reader tests; compile migrated examples.

### Phase 3: ZigZag utilities

**Proposed commit:** `Checked binary primitives phase 3: add explicit ZigZag codecs.`

Add transforms, checked readers and writer conveniences; document the distinction
from protobuf signed-int encoding. Test 0, +/-1, INT32/64_MIN/MAX, round trips,
uint32 overflow rejection, low-level output preservation and a golden byte
sequence for negative one. Verify narrower ZigZag overflow latches the reader's
data-error flag while transport status remains kOk, and later consuming calls
are suppressed. Run the relevant data read/write and typed-wrapper tests under
undefined-behavior sanitization.

### Phase 4: Strict UTF-8 predicate

**Proposed commit:** `Checked binary primitives phase 4: add strict UTF-8 validation.`

Port the original validator and add a programming-guide example. Validate scalar
boundary codepoints, embedded NUL, invalid leaders/continuations, all truncation
lengths, overlong forms, surrogate encodings, above-U+10FFFF and valid
noncharacters. Run //test/text:unicode_test and confirm decoder behavior is
unchanged.

### Phase 5: Borrowed limited input

**Proposed commit:** `Checked binary primitives phase 5: add borrowed input windows.`

Implement both adapters and document cursor exclusivity, no-drain destruction,
remaining counts and exact-boundary behavior. Add focused core tests for memory,
short streams, nested windows, transport errors, readFully, skip, zero-length
operations, nonblocking no-progress and synchronous contract violations.

Demonstrate that a child's exhaustion/close/destruction leaves the source open
and that the next frame's sentinel remains readable. Test through an existing
buffered iterator without losing its read-ahead. Assert no allocation in adapters
and record sizeof on the test ABI. Run the new core targets and affected existing
buffered-iterator tests.

### Phase 6: Counting output

**Proposed commit:** `Checked binary primitives phase 6: add bounded output counting.`

Implement the counting iterator and add a sizing example to the guide. Exercise
varint, fixed-width and byte-array writers against it; compare counts with actual
memory output. Cover zero writes, null bulk data, exact capacity, exceeded
capacity and arithmetic overflow near SIZE_MAX without allocating large buffers.
Confirm sticky failure, unchanged count on failed writes and no heap allocation.
Run the new core target and //test/data:write_test.

### Phase 7: roo_proto adoption (separate repository handoff)

**Proposed commit:** `Checked binary primitives phase 7: reuse roo_io codecs in roo_proto.`

After phases 1–6 land, update roo_proto's roo_io dependency and replace duplicate
varint validation, ZigZag, UTF-8, memory adaptation and counting operations.
Compose limited input without changing protocol Reader/Writer contracts or
introducing heap allocation. Delete duplicated implementation tests only after
the equivalent primitive tests exist in roo_io; keep protocol regression tests.

Run roo_proto's portable sanitizer suite, generated-code tests, framed/short-I/O
tests, zero-allocation test, both examples and the optional external Google
interoperability suite. Update its design, dependency manifests and validation
record. This is a **separate commit in roo_proto**, never bundled into roo_io's
commits. A roo_io-only delegation ends after phase 6 and records this remaining
consumer handoff; it does not silently modify the sibling repository.

## Testing Plan

Start with each phase's focused targets, then run the affected roo_io data, core
and text suites using its existing roo_testing profiles. Use sanitizers for
malformed data, integer arithmetic and boundary access. Keep successful legacy
behavior covered throughout the migration. New APIs must compile in the same
language baseline as roo_io; do not raise it to roo_proto's C++17 requirement.

Bazel outputs, caches, sandboxes and symlink prefixes must remain on persistent
storage under /home/dawidk, never /tmp. Preserve ~/.bazelrc disk-cache and resource
limits. Serialize memory-intensive Bazel commands across agents. Use the stable
workspace output location; use --batch or shut down a one-off server after its
last command. Do not change unrelated files or discard existing local work.

Completion evidence: focused commit hashes, passing target names/profiles,
formatting checks, documented public contracts, measured adapter sizes, and any
explicitly untested target environments. For a full two-repository assignment,
include the separate roo_proto migration commit and its regression results.

## Caveats

The string API change is source-breaking for low-level callers. Reader methods
retain value returns but now reject oversized fields by default; callers that
want the former truncating behavior for copied strings must select the
Truncated method. Views now return the complete field or fail. C-string
callers must supply capacity explicitly. Reader data_error_ includes violations
of the caller's size policy, not only intrinsically malformed wire encodings.

The deprecated low-level value-returning overload cannot distinguish zero from
failure. Its deprecation warning can affect builds treating warnings as errors;
migrate calls to the checked overload. The reader.readVarU64() method is not
deprecated. Use its bool counterpart for reliable direct decoding. Stateful
readers retain convenient value-returning calls and sticky data-error reporting.
Existing callers that check only reader.status() must migrate to reader.ok()
where malformed input needs to be detected: transport status alone intentionally
remains kOk after a data error.

The reader's post-error results have unspecified contents, not undefined
behavior. This does not weaken the checked low-level output argument's guarantee
of remaining unchanged on failure. Do not introduce uninitialized returns or
extra fast-path checks to manufacture a stronger sentinel guarantee.

EOF before or during a varint produces false with input.status() == kEndOfStream.
A framed/message parser supplies protocol-level truncation diagnostics. No byte
count or clean-EOF discriminator is added in this change.

Bounded input restricts reads through the adapter, not access through aliases.
Applications must respect cursor exclusivity. Arbitrary callback CPU work is not
bounded by these adapters. Size counting does not make a destructive producer
replayable; message encoding policy remains in roo_proto.

### Rejected Alternatives

#### Adding kBadData or a separate DecodeStatus

Keep transport status consistent between a low-level checked decoder and its
stateful reader. Bool plus the iterator status classifies a low-level failure;
the reader retains data-error state separately. No Status enum change is needed.

#### Another iterator adapter solely for sticky decoding errors

The reader already provides the adaptation boundary. Direct data_error_ guards
are sufficient; an additional abstraction adds complexity without a new need.

#### Checking ok() before every consuming reader call

Underlying iterators already suppress reads after transport failure. Check only
the new sticky data_error_ flag on the reader's ordinary fast path. Query input
status when classifying a failed checked decode, and expose ok() for callers.

#### Rejecting every non-minimal varint

Confuses canonical output with acceptable input and changes interoperability.
Only structural invalidity and overflow are rejected.

#### Moving roo_proto Reader/Writer wholesale

Would move protobuf tags, callback behavior and message budgets into a general
I/O library. Extract primitives and keep the protocol facade.

#### Per-field buffered stream readers

Can allocate, close borrowed streams, and strand read-ahead. Borrow existing
cursors and keep field windows unbuffered.

## Future Work

A stateful UTF-8 validator for chunked text,
additional framed-record abstractions and moving bounded containers to
roo_collections are separate proposals. They are not prerequisites for this
extraction.
