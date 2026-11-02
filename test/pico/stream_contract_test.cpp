#include <Stream.h>

#include <algorithm>
#include <vector>

#include "gtest/gtest.h"
#include "roo_io/stream/arduino/stream_input_stream.h"
#include "roo_io/stream/arduino/stream_output_stream.h"

namespace {
class ScriptedStream : public Stream {
 public:
  int available() override { return input.size() - position; }
  int read() override {
    if (position < input.size()) return input[position++];
    ++empty_reads;
    return -1;
  }
  int peek() override { return position < input.size() ? input[position] : -1; }
  int availableForWrite() override { return capacity; }
  size_t write(uint8_t value) override { return write(&value, 1); }
  size_t write(const uint8_t* data, size_t size) override {
    ++calls;
    const size_t written = std::min(size, limit);
    output.insert(output.end(), data, data + written);
    return written;
  }
  void flush() override {
    ++flushes;
    if (fail_flush) setWriteError();
  }
  std::vector<uint8_t> input{1, 2, 3};
  std::vector<uint8_t> output;
  size_t position = 0;
  size_t limit = 2;
  int capacity = 8;
  int calls = 0;
  int flushes = 0;
  int empty_reads = 0;
  bool fail_flush = false;
};

// Verifies zero operations have no side effects and writeFully handles partial
// writes without modifying the source, then flush propagates backend errors.
TEST(ArduinoStreamContract, OutputProgressAndFlush) {
  ScriptedStream stream;
  roo_io::ArduinoStreamOutputStream output(stream);
  const roo_io::byte data[] = {roo_io::byte{1}, roo_io::byte{2},
                               roo_io::byte{3}};
  EXPECT_EQ(output.write(nullptr, 0), 0u);
  EXPECT_EQ(stream.calls, 0);
  ASSERT_EQ(output.writeFully(data, 3), 3u);
  EXPECT_EQ(stream.calls, 2);
  EXPECT_EQ(stream.output, (std::vector<uint8_t>{1, 2, 3}));
  output.flush();
  EXPECT_EQ(stream.flushes, 1);
  stream.fail_flush = true;
  output.flush();
  EXPECT_EQ(output.status(), roo_io::kWriteError);
  EXPECT_EQ(output.write(data, 1), 0u);
}

// Verifies unavailable output is nonblocking, blocking write requests one
// byte, and zero progress terminates writeFully with an observable error.
TEST(ArduinoStreamContract, ZeroProgress) {
  ScriptedStream stream;
  stream.capacity = 0;
  roo_io::ArduinoStreamOutputStream output(stream);
  const roo_io::byte data{7};
  EXPECT_EQ(output.tryWrite(&data, 1), 0u);
  EXPECT_EQ(output.status(), roo_io::kOk);
  EXPECT_EQ(output.write(&data, 1), 1u);
  stream.limit = 0;
  EXPECT_EQ(output.writeFully(&data, 1), 0u);
  EXPECT_EQ(output.status(), roo_io::kWriteError);
}

// Verifies input reads bytes in order, zero/null handling, and closing leaves
// the borrowed underlying stream untouched.
TEST(ArduinoStreamContract, InputAndInvalidBuffers) {
  ScriptedStream stream;
  roo_io::ArduinoStreamInputStream input(stream);
  roo_io::byte bytes[4];
  EXPECT_EQ(input.tryRead(nullptr, 0), 0u);
  EXPECT_EQ(input.readFully(bytes, 3), 3u);
  EXPECT_EQ(bytes[2], roo_io::byte{3});
  EXPECT_EQ(input.tryRead(bytes, 1), 0u);
  EXPECT_EQ(input.read(nullptr, 1), 0u);
  EXPECT_EQ(input.status(), roo_io::kReadError);
  input.close();
  EXPECT_EQ(input.status(), roo_io::kClosed);
}

// Verifies tryRead respects output capacity and the available prefix without
// attempting reads that could enter Stream's timeout polling on an empty input.
TEST(ArduinoStreamContract, TryReadRespectsCapacityAndAvailability) {
  ScriptedStream stream;
  roo_io::ArduinoStreamInputStream input(stream);
  roo_io::byte bytes[4]{};
  ASSERT_EQ(input.tryRead(bytes, 2), 2u);
  EXPECT_EQ(bytes[0], roo_io::byte{1});
  EXPECT_EQ(bytes[1], roo_io::byte{2});
  EXPECT_EQ(bytes[2], roo_io::byte{0});
  ASSERT_EQ(input.tryRead(bytes + 2, 2), 1u);
  EXPECT_EQ(bytes[2], roo_io::byte{3});
  EXPECT_EQ(bytes[3], roo_io::byte{0});
  EXPECT_EQ(input.tryRead(bytes, sizeof(bytes)), 0u);
  EXPECT_EQ(input.status(), roo_io::kOk);
  EXPECT_EQ(stream.empty_reads, 0);
}

}  // namespace
