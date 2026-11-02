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
