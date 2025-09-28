#include "roo_io/pipe/pipe.h"

#include "roo_threads/thread.h"

#include "gtest/gtest.h"

namespace roo_io {

TEST(PipeTest, BasicWriteRead) {
  Pipe pipe(4);
  byte data[] = {byte{1}, byte{2}, byte{3}};
  size_t written = pipe.write(data, 3);
  EXPECT_EQ(written, 3);
  EXPECT_EQ(pipe.availableForRead(), 3);
  EXPECT_EQ(pipe.availableForWrite(), 1);

  byte out[3];
  size_t read = pipe.read(out, 3);
  EXPECT_EQ(read, 3);
  EXPECT_EQ(out[0], byte{1});
  EXPECT_EQ(out[1], byte{2});
  EXPECT_EQ(out[2], byte{3});
  EXPECT_EQ(pipe.availableForRead(), 0);
  EXPECT_EQ(pipe.availableForWrite(), 4);
}

TEST(PipeTest, WriteMoreThanCapacity) {
  Pipe pipe(2);
  byte data[] = {byte{1}, byte{2}, byte{3}};
  size_t written = pipe.write(data, 3);
  EXPECT_EQ(written, 2);
  EXPECT_EQ(pipe.availableForRead(), 2);
  EXPECT_EQ(pipe.availableForWrite(), 0);
}

TEST(PipeTest, CloseUnblocksRead) {
  Pipe pipe(2);
  byte data[] = {byte{1}, byte{2}};
  size_t written = pipe.write(data, 2);
  EXPECT_EQ(written, 2);

  pipe.closeOutput();
  EXPECT_EQ(pipe.outputStatus(), kClosed);

  byte out[2];
  size_t read = pipe.read(out, 2);
  EXPECT_EQ(read, 2);
  EXPECT_EQ(out[0], byte{1});
  EXPECT_EQ(out[1], byte{2});

  read = pipe.read(out, 2);
  EXPECT_EQ(read, 0);  // No more data.
  EXPECT_EQ(0, pipe.availableForRead());
  EXPECT_EQ(0, pipe.availableForWrite());
}

TEST(PipeTest, CloseAwakesReadAsync) {
  Pipe pipe(2);
  byte out[2];

  roo::thread reader([&pipe, &out]() {
    size_t read = pipe.read(out, 2);
    EXPECT_EQ(read, 0);  // Should return immediately after close.
  });

  // Give the reader a moment to start and block on read.
  roo::this_thread::sleep_for(roo_time::Millis(100));
  pipe.closeOutput();
  EXPECT_EQ(pipe.outputStatus(), kClosed);
  EXPECT_EQ(0, pipe.availableForRead());
  EXPECT_EQ(0, pipe.availableForWrite());

  reader.join();
}

TEST(PipeTest, ClosingInputPreventsFurtherWrites) {
  Pipe pipe(2);
  pipe.closeInput();
  EXPECT_EQ(pipe.inputStatus(), kClosed);

  byte data[] = {byte{1}, byte{2}};
  size_t written = pipe.write(data, 2);
  EXPECT_EQ(written, 0);  // Cannot write after input is closed.
  EXPECT_EQ(pipe.outputStatus(), kBrokenPipe);
  EXPECT_EQ(0, pipe.availableForRead());
  EXPECT_EQ(0, pipe.availableForWrite());
}

}  // namespace roo_io