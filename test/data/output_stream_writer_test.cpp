#include "roo_io/data/output_stream_writer.h"

#include <memory>

#include "gtest/gtest.h"
#include "roo_io/core/output_stream.h"

namespace roo_io {
namespace {

class CountingOutputStream : public OutputStream {
 public:
  static void Reset() {
    destroyed_ = 0;
    closed_ = 0;
  }

  static int destroyed() { return destroyed_; }
  static int closed() { return closed_; }

  ~CountingOutputStream() override { ++destroyed_; }

  size_t write(const byte* /*buf*/, size_t count) override {
    if (status_ != kOk) return 0;
    return count;
  }

  void close() override {
    if (status_ == kOk) {
      status_ = kClosed;
    }
    ++closed_;
  }

  Status status() const override { return status_; }

 private:
  Status status_ = kOk;
  static int destroyed_;
  static int closed_;
};

int CountingOutputStream::destroyed_ = 0;
int CountingOutputStream::closed_ = 0;

class FaultyOutputStream : public OutputStream {
 public:
  FaultyOutputStream(Status write_status, Status close_status)
      : write_status_(write_status), close_status_(close_status) {}

  size_t write(const byte* /*buf*/, size_t count) override {
    status_ = write_status_;
    return status_ == kOk ? count : 0;
  }

  void flush() override { status_ = kOk; }

  void close() override { status_ = close_status_; }

  Status status() const override { return status_; }

 private:
  Status write_status_;
  Status close_status_;
  Status status_ = kOk;
};

TEST(OutputStreamWriterTest, CloseDeletesOwnedStream) {
  CountingOutputStream::Reset();
  {
    OutputStreamWriter writer(
        std::unique_ptr<OutputStream>(new CountingOutputStream()));
    writer.writeU8(0x11);
    EXPECT_EQ(0, CountingOutputStream::destroyed());
    writer.close();
    EXPECT_EQ(kClosed, writer.status());
    EXPECT_EQ(1, CountingOutputStream::destroyed());
    EXPECT_EQ(1, CountingOutputStream::closed());
  }
  EXPECT_EQ(1, CountingOutputStream::destroyed());
}

TEST(OutputStreamWriterTest, ResetDeletesPreviousOwnedStream) {
  CountingOutputStream::Reset();
  OutputStreamWriter writer(
      std::unique_ptr<OutputStream>(new CountingOutputStream()));
  writer.reset(std::unique_ptr<OutputStream>(new CountingOutputStream()));
  EXPECT_EQ(1, CountingOutputStream::destroyed());
  writer.close();
  EXPECT_EQ(2, CountingOutputStream::destroyed());
  EXPECT_EQ(2, CountingOutputStream::closed());
}

// Verifies an error during the buffered write survives close and remains
// visible until a new stream is bound.
TEST(OutputStreamWriterTest, ClosePreservesBufferedWriteError) {
  FaultyOutputStream failing(kNoSpaceLeftOnDevice, kWriteError);
  CountingOutputStream replacement;
  OutputStreamWriter writer(failing);
  writer.writeU8(0x11);
  ASSERT_TRUE(writer.ok());
  writer.close();
  EXPECT_EQ(kNoSpaceLeftOnDevice, writer.status());
  writer.close();
  EXPECT_EQ(kNoSpaceLeftOnDevice, writer.status());
  writer.reset(replacement);
  EXPECT_TRUE(writer.ok());
}

// Verifies a failure reported only by the underlying close reaches the caller
// after the owned stream has been released.
TEST(OutputStreamWriterTest, ClosePreservesOwnedStreamCloseError) {
  OutputStreamWriter writer(
      std::make_unique<FaultyOutputStream>(kOk, kWriteError));
  writer.writeU8(0x11);
  writer.close();
  EXPECT_EQ(kWriteError, writer.status());
}

}  // namespace
}  // namespace roo_io
