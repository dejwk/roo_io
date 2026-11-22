#include "roo_io/data/output_stream_writer.h"

#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "roo_io/core/output_stream.h"

namespace roo_io {
namespace {

class CountingOutputStream : public OutputStream {
 public:
  explicit CountingOutputStream(std::vector<byte>* writes = nullptr)
      : writes_(writes) {}

  static void Reset() {
    destroyed_ = 0;
    closed_ = 0;
  }

  static int destroyed() { return destroyed_; }
  static int closed() { return closed_; }

  ~CountingOutputStream() override { ++destroyed_; }

  size_t write(const byte* buf, size_t count) override {
    if (status_ != kOk) return 0;
    if (writes_ != nullptr && count != 0) {
      writes_->insert(writes_->end(), buf, buf + count);
    }
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
  std::vector<byte>* writes_;
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

// Verifies that replacing an owned stream writes its buffered data before
// closing and destroying it.
TEST(OutputStreamWriterTest, ResetFlushesAndDeletesPreviousOwnedStream) {
  CountingOutputStream::Reset();
  std::vector<byte> writes;
  OutputStreamWriter writer(std::make_unique<CountingOutputStream>(&writes));
  writer.writeU8(0x11);
  writer.reset(std::make_unique<CountingOutputStream>());
  EXPECT_EQ((std::vector<byte>{byte{0x11}}), writes);
  EXPECT_EQ(1, CountingOutputStream::destroyed());
  writer.close();
  EXPECT_EQ(2, CountingOutputStream::destroyed());
  EXPECT_EQ(2, CountingOutputStream::closed());
}

// Verifies that buffered data goes to the original borrowed stream and
// subsequent writes go to the replacement.
TEST(OutputStreamWriterTest, ResetFlushesPreviousBorrowedStream) {
  std::vector<byte> first_writes;
  std::vector<byte> second_writes;
  CountingOutputStream first(&first_writes);
  CountingOutputStream second(&second_writes);
  OutputStreamWriter writer(first);
  writer.writeU8(0x11);
  writer.reset(second);
  EXPECT_EQ((std::vector<byte>{byte{0x11}}), first_writes);
  EXPECT_EQ(kClosed, first.status());
  writer.writeU8(0x22);
  writer.close();
  EXPECT_EQ((std::vector<byte>{byte{0x22}}), second_writes);
}

// Verifies that adopting the borrowed stream preserves its lifetime and buffer.
TEST(OutputStreamWriterTest, ResetAdoptsCurrentBorrowedStream) {
  CountingOutputStream::Reset();
  std::vector<byte> writes;
  auto stream = std::make_unique<CountingOutputStream>(&writes);
  OutputStreamWriter writer(*stream);
  writer.writeU8(0x11);
  writer.reset(std::move(stream));
  EXPECT_EQ(0, CountingOutputStream::destroyed());
  EXPECT_TRUE(writes.empty());
  writer.writeU8(0x22);
  writer.close();
  EXPECT_EQ((std::vector<byte>{byte{0x11}, byte{0x22}}), writes);
  EXPECT_EQ(1, CountingOutputStream::destroyed());
  EXPECT_EQ(1, CountingOutputStream::closed());
}

// Verifies that detaching via reset reports errors from flushing pending data.
TEST(OutputStreamWriterTest, ResetToNullPreservesWriteError) {
  FaultyOutputStream failing(kNoSpaceLeftOnDevice, kWriteError);
  OutputStreamWriter writer(failing);
  writer.writeU8(0x11);
  writer.reset(nullptr);
  EXPECT_EQ(kNoSpaceLeftOnDevice, writer.status());
  writer.reset(nullptr);
  EXPECT_EQ(kNoSpaceLeftOnDevice, writer.status());
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
