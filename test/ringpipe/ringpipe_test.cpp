#include "roo_io/ringpipe/ringpipe.h"

#include "gtest/gtest.h"
#include "roo_io/ringpipe/ringpipe_input_stream.h"
#include "roo_io/ringpipe/ringpipe_output_stream.h"
#include "roo_threads/thread.h"

namespace roo_io {

TEST(RingPipe, BasicWriteRead) {
  RingPipe pipe(4);
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

TEST(RingPipe, WriteMoreThanCapacity) {
  RingPipe pipe(2);
  byte data[] = {byte{1}, byte{2}, byte{3}};
  size_t written = pipe.write(data, 3);
  EXPECT_EQ(written, 2);
  EXPECT_EQ(pipe.availableForRead(), 2);
  EXPECT_EQ(pipe.availableForWrite(), 0);
}

TEST(RingPipe, CloseUnblocksRead) {
  RingPipe pipe(2);
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

TEST(RingPipe, CloseAwakesReadAsync) {
  RingPipe pipe(2);
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

TEST(RingPipe, ClosingInputPreventsFurtherWrites) {
  RingPipe pipe(2);
  pipe.closeInput();
  EXPECT_EQ(pipe.inputStatus(), kClosed);

  byte data[] = {byte{1}, byte{2}};
  size_t written = pipe.write(data, 2);
  EXPECT_EQ(written, 0);  // Cannot write after input is closed.
  EXPECT_EQ(pipe.outputStatus(), kBrokenPipe);
  EXPECT_EQ(0, pipe.availableForRead());
  EXPECT_EQ(0, pipe.availableForWrite());
}

TEST(RingPipe, BasicStreams) {
  RingPipe pipe(128);
  RingPipeOutputStream output_stream(pipe);
  RingPipeInputStream input_stream(pipe);
  EXPECT_TRUE(output_stream.isOpen());
  EXPECT_TRUE(input_stream.isOpen());
  EXPECT_EQ(output_stream.status(), kOk);
  EXPECT_EQ(input_stream.status(), kOk);
  EXPECT_EQ(output_stream.availableForWrite(), 128);
  EXPECT_EQ(input_stream.available(), 0);
  byte data[] = {byte{1}, byte{2}, byte{3}};
  size_t written = output_stream.write(data, 3);
  EXPECT_EQ(written, 3);
  EXPECT_EQ(output_stream.availableForWrite(), 125);
  EXPECT_EQ(input_stream.available(), 3);
  byte out[3];
  size_t read = input_stream.read(out, 3);
  EXPECT_EQ(read, 3);
  EXPECT_EQ(out[0], byte{1});
  EXPECT_EQ(out[1], byte{2});
  EXPECT_EQ(out[2], byte{3});
  EXPECT_EQ(output_stream.availableForWrite(), 128);
  EXPECT_EQ(input_stream.available(), 0);
  output_stream.close();
  EXPECT_FALSE(output_stream.isOpen());
  EXPECT_EQ(output_stream.status(), kClosed);
  EXPECT_TRUE(input_stream.isOpen());
  EXPECT_EQ(input_stream.status(), kEndOfStream);
  input_stream.close();
  EXPECT_FALSE(input_stream.isOpen());
  EXPECT_EQ(input_stream.status(), kClosed);
}

TEST(RingPipe, StreamsDataTransfer) {
  RingPipe pipe(128);
  RingPipeOutputStream output_stream(pipe);
  RingPipeInputStream input_stream(pipe);
  const size_t data_size = 1000000;
  byte* data = new byte[data_size];
  for (size_t i = 0; i < data_size; ++i) {
    data[i] = (byte)(i & 0xFF);
  }
  roo::thread writer([&output_stream, data, data_size]() {
    size_t total_written = 0;
    while (total_written < data_size) {
      size_t written =
          output_stream.write(data + total_written, data_size - total_written);
      if (written > 0) {
        total_written += written;
      }
      roo::this_thread::yield();
    }
    output_stream.close();
  });
  byte* in = new byte[data_size];
  size_t total_read = 0;
  while (total_read < data_size) {
    size_t read = input_stream.read(in + total_read, data_size - total_read);
    if (read == 0) break;
    total_read += read;
    roo::this_thread::yield();
  }
  EXPECT_EQ(input_stream.status(), kEndOfStream);
  input_stream.close();
  writer.join();
  EXPECT_EQ(total_read, data_size);
  for (size_t i = 0; i < data_size; ++i) {
    EXPECT_EQ(in[i], byte{i & 0xFF});
  }
  delete[] data;
  delete[] in;
}

}  // namespace roo_io