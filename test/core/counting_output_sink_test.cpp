#include "roo_io/core/counting_output_sink.h"

#include <limits>

#include "gtest/gtest.h"
#include "roo_io/data/write.h"

namespace roo_io {

// Verifies fixed-width and varint writers can size an encoded record without a
// backing buffer.
TEST(CountingOutputSink, CountsEncodedWrites) {
  CountingOutputSink output;
  WriteBeU16(output, 0x1234);
  WriteVarU64(output, 150);
  WriteString(output, "hi");
  EXPECT_EQ(7U, output.size());
  EXPECT_EQ(kOk, output.status());
}

// Verifies over-limit writes fail atomically and remain sticky.
TEST(CountingOutputSink, EnforcesLimit) {
  CountingOutputSink output(3);
  const byte* unavailable = nullptr;
  EXPECT_EQ(3U, output.write(unavailable, 3));
  EXPECT_EQ(3U, output.size());
  EXPECT_EQ(0U, output.write(unavailable, 1));
  EXPECT_EQ(3U, output.size());
  EXPECT_EQ(kNoSpaceLeftOnDevice, output.status());
  output.write(byte{0});
  EXPECT_EQ(3U, output.size());
}

// Verifies zero-length bulk writes do not alter an exhausted sink.
TEST(CountingOutputSink, ZeroLengthWriteIsNoOp) {
  CountingOutputSink output(0);
  const byte* unavailable = nullptr;
  EXPECT_EQ(0U, output.write(unavailable, 0));
  EXPECT_EQ(kOk, output.status());
  output.write(byte{0});
  EXPECT_EQ(kNoSpaceLeftOnDevice, output.status());
}

// Verifies capacity arithmetic cannot wrap when a caller supplies only a
// length and no backing buffer.
TEST(CountingOutputSink, DetectsSizeOverflow) {
  CountingOutputSink output;
  const byte* unavailable = nullptr;
  EXPECT_EQ(std::numeric_limits<size_t>::max() - 1,
            output.write(unavailable, std::numeric_limits<size_t>::max() - 1));
  EXPECT_EQ(0U, output.write(unavailable, 2));
  EXPECT_EQ(std::numeric_limits<size_t>::max() - 1, output.size());
  EXPECT_EQ(kNoSpaceLeftOnDevice, output.status());
}

}  // namespace roo_io
