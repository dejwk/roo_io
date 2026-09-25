#include "gtest/gtest.h"
#include "roo_io/core/limited_input_iterator.h"
#include "roo_io/core/limited_input_stream.h"
#include "roo_io/memory/memory_input_iterator.h"
#include "roo_io/memory/memory_input_stream.h"

namespace roo_io {

namespace {

struct ObservedInput {
  byte read() {
    ++calls;
    state = error;
    return byte{7};
  }
  size_t read(byte*, size_t count) {
    ++calls;
    state = error;
    return stalled ? 0 : (count > 1 ? 1 : count);
  }
  Status status() const {
    ++queries;
    return state;
  }
  mutable unsigned queries = 0;
  unsigned calls = 0;
  bool stalled = false;
  Status state = kOk;
  Status error = kOk;
};

}  // namespace

// Verifies one status sample per transfer and none for cached queries or
// errors.
TEST(LimitedInputIterator, CachesStatusAndCountsSuccessfulBytes) {
  ObservedInput source;
  LimitedInputIterator<ObservedInput> input(source, 5);
  EXPECT_EQ(1U, source.queries);
  EXPECT_EQ(byte{7}, input.read());
  EXPECT_EQ(kOk, input.status());
  EXPECT_EQ(2U, source.queries);
  source.error = kReadError;
  EXPECT_EQ(byte{0}, input.read());
  EXPECT_EQ(4U, input.remaining());
  EXPECT_EQ(kReadError, input.status());
  input.read();
  EXPECT_EQ(3U, source.queries);
  EXPECT_EQ(2U, source.calls);
}

// Verifies bulk partial failures count the prefix and zero progress is an
// error.
TEST(LimitedInputIterator, BulkErrorsAndInitialStatus) {
  for (bool stalled : {false, true}) {
    ObservedInput source;
    source.stalled = stalled;
    source.error = stalled ? kOk : kEndOfStream;
    LimitedInputIterator<ObservedInput> input(source, 5);
    byte data[5];
    EXPECT_EQ(stalled ? 0U : 1U, input.read(data, 5));
    EXPECT_EQ(stalled ? 5U : 4U, input.remaining());
    EXPECT_EQ(stalled ? kReadError : kEndOfStream, input.status());
    EXPECT_EQ(0U, input.read(data, 1));
    EXPECT_EQ(2U, source.queries);
    EXPECT_EQ(1U, source.calls);
  }
  ObservedInput source;
  source.state = kReadError;
  LimitedInputIterator<ObservedInput> input(source, 5);
  input.read();
  EXPECT_EQ(kReadError, input.status());
  EXPECT_EQ(0U, source.calls);
}

// Verifies a limited iterator stops at its local boundary without consuming the
// next record from the borrowed cursor.
TEST(LimitedInputIterator, PreservesFollowingRecord) {
  const byte data[] = {byte{1}, byte{2}, byte{3}};
  MemoryIterator source(data, data + 3);
  LimitedInputIterator<MemoryIterator> input(source, 2);
  EXPECT_EQ(byte{1}, input.read());
  EXPECT_EQ(byte{2}, input.read());
  EXPECT_EQ(0U, input.remaining());
  EXPECT_EQ(kOk, input.status());
  EXPECT_EQ(byte{0}, input.read());
  EXPECT_EQ(kEndOfStream, input.status());
  EXPECT_EQ(kOk, source.status());
  EXPECT_EQ(byte{3}, source.read());
}

// Verifies bulk reads clamp to the local boundary and zero-length reads do not
// turn an exhausted window into EOF.
TEST(LimitedInputIterator, ClampsBulkReads) {
  const byte data[] = {byte{1}, byte{2}, byte{3}};
  MemoryIterator source(data, data + 3);
  LimitedInputIterator<MemoryIterator> input(source, 2);
  byte result[3] = {};
  EXPECT_EQ(2U, input.read(result, 3));
  EXPECT_EQ(byte{1}, result[0]);
  EXPECT_EQ(byte{2}, result[1]);
  EXPECT_EQ(kOk, input.status());
  EXPECT_EQ(0U, input.read(result, 0));
  EXPECT_EQ(kOk, input.status());
}

// Verifies a limited stream borrows its source and reports local exhaustion.
TEST(LimitedInputStream, PreservesSourceAndSupportsReadFully) {
  const byte data[] = {byte{1}, byte{2}, byte{3}};
  MemoryInputStream source(data, data + 3);
  LimitedInputStream input(source, 2);
  byte result[3] = {};
  EXPECT_EQ(2U, input.readFully(result, 3));
  EXPECT_EQ(byte{1}, result[0]);
  EXPECT_EQ(byte{2}, result[1]);
  EXPECT_EQ(kEndOfStream, input.status());
  EXPECT_EQ(kOk, source.status());
  EXPECT_EQ(1U, source.read(result, 1));
  EXPECT_EQ(byte{3}, result[0]);
}

// Verifies closing a borrowed window does not close its source stream.
TEST(LimitedInputStream, CloseDoesNotCloseSource) {
  const byte data[] = {byte{1}};
  MemoryInputStream source(data, data + 1);
  LimitedInputStream input(source, 1);
  input.close();
  EXPECT_EQ(kClosed, input.status());
  EXPECT_EQ(kOk, source.status());
  byte result = byte{0};
  EXPECT_EQ(1U, source.read(&result, 1));
  EXPECT_EQ(byte{1}, result);
}

}  // namespace roo_io
