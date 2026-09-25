#include "roo_io/core/limited_output_iterator.h"

#include "gtest/gtest.h"
#include "roo_io/core/counting_output_iterator.h"

namespace roo_io {
namespace {

struct Sink {
  void write(byte) {
    ++calls;
    if (error != kOk) state = error;
    if (state == kOk) ++size;
  }
  size_t write(const byte*, size_t count) {
    ++calls;
    size_t written = stalled ? 0 : (count > 2 ? 2 : count);
    size += written;
    state = error;
    return written;
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
  size_t size = 0;
  bool stalled = false;
  Status error = kOk;
  Status state = kOk;
};

// Verifies short writes are reported and only transfers query sink status.
TEST(LimitedOutputIterator, ShortWritesAndCachedStatus) {
  Sink sink;
  LimitedOutputIterator<Sink> output(sink, 6);
  byte data[5] = {};
  EXPECT_EQ(2U, output.write(data, 5));
  EXPECT_EQ(2U, output.write(data + 2, 3));
  EXPECT_EQ(1U, output.write(data + 4, 1));
  EXPECT_EQ(3U, sink.calls);
  EXPECT_EQ(4U, sink.queries);
  EXPECT_EQ(1U, output.remaining());
  EXPECT_EQ(kOk, output.status());
  output.write(byte{1});
  EXPECT_EQ(5U, sink.queries);
  EXPECT_EQ(kOk, output.status());
  EXPECT_EQ(0U, output.write(data, 0));
  output.write(byte{1});
  EXPECT_EQ(kNoSpaceLeftOnDevice, output.status());
  EXPECT_EQ(5U, sink.queries);
  EXPECT_EQ(6U, sink.size);
  EXPECT_EQ(kOk, sink.state);
}

// Verifies oversized requests are atomic and do not poison the borrowed sink.
TEST(LimitedOutputIterator, RejectsOversizedBulk) {
  Sink sink;
  LimitedOutputIterator<Sink> output(sink, 1);
  byte data[2] = {};
  EXPECT_EQ(0U, output.write(data, 2));
  EXPECT_EQ(kNoSpaceLeftOnDevice, output.status());
  EXPECT_EQ(1U, output.remaining());
  EXPECT_EQ(0U, sink.calls);
}

// Verifies partial errors, zero progress and initial errors become sticky.
TEST(LimitedOutputIterator, StopsOnErrors) {
  byte data[5] = {};
  for (bool stalled : {false, true}) {
    Sink sink;
    sink.stalled = stalled;
    sink.error = stalled ? kOk : kWriteError;
    LimitedOutputIterator<Sink> output(sink, 5);
    EXPECT_EQ(stalled ? 0U : 2U, output.write(data, 5));
    EXPECT_EQ(stalled ? 5U : 3U, output.remaining());
    EXPECT_EQ(kWriteError, output.status());
    output.write(byte{1});
    output.flush();
    EXPECT_EQ(1U, sink.calls);
    EXPECT_EQ(0U, sink.flushes);
  }
  Sink sink;
  sink.state = kWriteError;
  LimitedOutputIterator<Sink> output(sink, 5);
  output.write(byte{1});
  EXPECT_EQ(kWriteError, output.status());
  EXPECT_EQ(0U, sink.calls);
}

// Verifies explicit flushing propagates errors and destruction never flushes.
TEST(LimitedOutputIterator, ExplicitFlush) {
  Sink sink;
  {
    LimitedOutputIterator<Sink> output(sink, 0);
    sink.error = kWriteError;
    output.flush();
    EXPECT_EQ(kWriteError, output.status());
  }
  EXPECT_EQ(1U, sink.flushes);
}

// Verifies accounting accepts counting sinks without pointer arithmetic
// overflow.
TEST(LimitedOutputIterator, CountingWithoutStorage) {
  CountingOutputIterator sink;
  LimitedOutputIterator<CountingOutputIterator> output(sink, SIZE_MAX);
  EXPECT_EQ(SIZE_MAX, output.write(nullptr, SIZE_MAX));
  EXPECT_EQ(0U, output.remaining());
  EXPECT_EQ(SIZE_MAX, sink.size());
}

}  // namespace
}  // namespace roo_io
