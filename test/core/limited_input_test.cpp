#include "gtest/gtest.h"
#include "roo_io/core/limited_input_iterator.h"
#include "roo_io/core/limited_input_stream.h"
#include "roo_io/memory/memory_input_iterator.h"
#include "roo_io/memory/memory_input_stream.h"

namespace roo_io {

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
