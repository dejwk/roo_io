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

TEST(OutputStreamWriterTest, CloseDeletesOwnedStream) {
  CountingOutputStream::Reset();
  {
    OutputStreamWriter writer(
        std::unique_ptr<OutputStream>(new CountingOutputStream()));
    writer.writeU8(0x11);
    EXPECT_EQ(0, CountingOutputStream::destroyed());
    writer.close();
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

}  // namespace
}  // namespace roo_io
