#include "gtest/gtest.h"
#include "roo_io/core/counting_input_iterator.h"
#include "roo_io/core/counting_output_iterator.h"
#include "roo_io/core/counting_output_sink.h"
#include "roo_io/data/read.h"
#include "roo_io/data/write.h"
#include "roo_io/memory/memory_input_iterator.h"
#include "roo_io/memory/memory_output_iterator.h"

namespace roo_io {
namespace {

// Allows partial transfers, initial failures and transfer-time failures.
struct ObservedIterator {
  byte read() {
    ++calls;
    state = error;
    return byte{7};
  }
  size_t read(byte*, size_t count) { return transfer(count); }
  void write(byte) {
    ++calls;
    state = error;
  }
  size_t write(const byte*, size_t count) { return transfer(count); }
  size_t transfer(size_t count) {
    ++calls;
    state = error;
    return stalled ? 0 : (count > 2 ? 2 : count);
  }
  void skip(size_t count) {
    ++skips;
    skipped = count;
    state = error;
  }
  void flush() {
    ++flushes;
    state = error;
  }
  Status status() const {
    ++queries;
    return state;
  }
  mutable unsigned queries = 0;
  unsigned calls = 0;
  unsigned flushes = 0;
  unsigned skips = 0;
  size_t skipped = 0;
  bool stalled = false;
  Status state = kOk;
  Status error = kOk;
};

// Verifies concrete memory iterators round-trip fixed fields through counters.
TEST(CountingIterator, FixedWidthRoundTrip) {
  byte data[12] = {};
  MemoryOutputIterator sink(data, data + 12);
  CountingOutputIterator<MemoryOutputIterator> output(sink);
  WriteLeU32(output, 0x12345678);
  WriteLeU64(output, UINT64_MAX);
  EXPECT_EQ(12U, output.size());
  EXPECT_EQ(kOk, output.status());
  MemoryIterator source(data, data + 12);
  CountingInputIterator<MemoryIterator> input(source);
  EXPECT_EQ(0x12345678U, ReadLeU32(input));
  EXPECT_EQ(UINT64_MAX, ReadLeU64(input));
  EXPECT_EQ(12U, input.size());
  input.read();
  EXPECT_EQ(kEndOfStream, input.status());
  EXPECT_EQ(12U, input.size());
}

// Verifies successful scalar reads query the source once, then cache errors.
TEST(CountingInputIterator, CachedStatus) {
  ObservedIterator source;
  CountingInputIterator<ObservedIterator> input(source);
  EXPECT_EQ(byte{7}, input.read());
  EXPECT_EQ(kOk, input.status());
  EXPECT_EQ(2U, source.queries);
  source.error = kReadError;
  EXPECT_EQ(byte{0}, input.read());
  input.read();
  EXPECT_EQ(kReadError, input.status());
  EXPECT_EQ(1U, input.size());
  EXPECT_EQ(3U, source.queries);
  EXPECT_EQ(2U, source.calls);
}

// Verifies bulk errors count the accepted prefix and zero progress is sticky.
TEST(CountingInputIterator, PartialFailureAndNoProgress) {
  for (bool stalled : {false, true}) {
    ObservedIterator source;
    source.stalled = stalled;
    source.error = stalled ? kOk : kEndOfStream;
    CountingInputIterator<ObservedIterator> input(source);
    byte data[5];
    EXPECT_EQ(0U, input.read(data, 0));
    EXPECT_EQ(stalled ? 0U : 2U, input.read(data, 5));
    EXPECT_EQ(stalled ? 0U : 2U, input.size());
    EXPECT_EQ(stalled ? kReadError : kEndOfStream, input.status());
    EXPECT_EQ(0U, input.read(data, 1));
    EXPECT_EQ(1U, source.calls);
    EXPECT_EQ(2U, source.queries);
  }
}

// Verifies skip forwards once without reading, counting only successful skips.
TEST(CountingInputIterator, DelegatesSkip) {
  ObservedIterator source;
  CountingInputIterator<ObservedIterator> input(source);
  input.skip(0);
  EXPECT_EQ(0U, source.skips);
  EXPECT_EQ(1U, source.queries);
  input.skip(1024);
  EXPECT_EQ(1024U, input.size());
  EXPECT_EQ(1024U, source.skipped);
  EXPECT_EQ(1U, source.skips);
  EXPECT_EQ(0U, source.calls);
  EXPECT_EQ(2U, source.queries);
  source.error = kEndOfStream;
  input.skip(2048);
  EXPECT_EQ(kEndOfStream, input.status());
  EXPECT_EQ(1024U, input.size());
  EXPECT_EQ(2U, source.skips);
  EXPECT_EQ(3U, source.queries);
  input.skip(1);
  EXPECT_EQ(2U, source.skips);
  EXPECT_EQ(3U, source.queries);
}

// Verifies a failed memory skip leaves the count at the preceding success.
TEST(CountingInputIterator, FailedSkipExcludesPartialProgress) {
  byte data[3] = {};
  MemoryIterator source(data, data + 3);
  CountingInputIterator<MemoryIterator> input(source);
  input.skip(2);
  EXPECT_EQ(2U, input.size());
  input.skip(2);
  EXPECT_EQ(2U, input.size());
  EXPECT_EQ(kEndOfStream, input.status());
}

// Verifies failed sources and sinks are never called.
TEST(CountingIterator, InitialError) {
  ObservedIterator iterator;
  iterator.state = kReadError;
  CountingInputIterator<ObservedIterator> input(iterator);
  input.read();
  EXPECT_EQ(kReadError, input.status());
  EXPECT_EQ(0U, input.size());
  iterator.state = kWriteError;
  CountingOutputIterator<ObservedIterator> output(iterator);
  output.write(byte{1});
  output.flush();
  EXPECT_EQ(kWriteError, output.status());
  EXPECT_EQ(0U, output.size());
  EXPECT_EQ(0U, iterator.calls);
  EXPECT_EQ(0U, iterator.flushes);
}

// Verifies scalar output counts only successes and caches transfer status.
TEST(CountingOutputIterator, CachedStatus) {
  ObservedIterator sink;
  CountingOutputIterator<ObservedIterator> output(sink);
  output.write(byte{1});
  EXPECT_EQ(kOk, output.status());
  EXPECT_EQ(2U, sink.queries);
  sink.error = kWriteError;
  output.write(byte{2});
  output.write(byte{3});
  EXPECT_EQ(kWriteError, output.status());
  EXPECT_EQ(1U, output.size());
  EXPECT_EQ(3U, sink.queries);
  EXPECT_EQ(2U, sink.calls);
}

// Verifies short output remains short, and partial errors retain their count.
TEST(CountingOutputIterator, ShortTransfersAndErrors) {
  byte data[5] = {};
  ObservedIterator sink;
  CountingOutputIterator<ObservedIterator> output(sink);
  EXPECT_EQ(2U, output.write(data, 5));
  EXPECT_EQ(kOk, output.status());
  sink.error = kWriteError;
  EXPECT_EQ(2U, output.write(data, 3));
  EXPECT_EQ(4U, output.size());
  EXPECT_EQ(kWriteError, output.status());
  EXPECT_EQ(0U, output.write(data, 1));
  EXPECT_EQ(2U, sink.calls);
}

// Verifies no-progress output fails and explicit flush forwards errors.
TEST(CountingOutputIterator, ZeroProgressAndFlush) {
  ObservedIterator sink;
  {
    CountingOutputIterator<ObservedIterator> output(sink);
    EXPECT_EQ(0U, output.write(nullptr, 0));
    EXPECT_EQ(0U, sink.calls);
    output.flush();
    EXPECT_EQ(kOk, output.status());
    sink.error = kWriteError;
    output.flush();
    EXPECT_EQ(kWriteError, output.status());
  }
  EXPECT_EQ(2U, sink.flushes);
  sink.state = kOk;
  sink.error = kOk;
  sink.stalled = true;
  CountingOutputIterator<ObservedIterator> output(sink);
  EXPECT_EQ(0U, output.write(nullptr, 1));
  EXPECT_EQ(kWriteError, output.status());
  EXPECT_EQ(0U, output.size());
}

// Verifies counting forwards even huge counts without touching the buffer.
TEST(CountingOutputIterator, NoBackingStorage) {
  CountingOutputSink sink;
  CountingOutputIterator<CountingOutputSink> output(sink);
  EXPECT_EQ(SIZE_MAX, output.write(nullptr, SIZE_MAX));
  EXPECT_EQ(SIZE_MAX, output.size());
}

}  // namespace
}  // namespace roo_io
