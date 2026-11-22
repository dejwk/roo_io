#include "roo_io/fs/posix/posix_file_output_stream.h"

#include <sys/stat.h>

#include <cstdio>
#include <memory>
#include <utility>

#include "gtest/gtest.h"
#include "roo_io/data/output_stream_writer.h"

namespace roo_io {
namespace {

// Verifies that flushing makes buffered bytes visible while the file is open.
TEST(PosixFileOutputStreamTest, FlushWritesBufferedData) {
  std::FILE* file = std::tmpfile();
  ASSERT_NE(nullptr, file);
  PosixFileOutputStream output(nullptr, file);
  ASSERT_EQ(0, std::setvbuf(file, nullptr, _IOFBF, BUFSIZ));

  const byte value{0x11};
  ASSERT_EQ(1, output.write(&value, 1));
  struct stat info = {};
  ASSERT_EQ(0, ::fstat(::fileno(file), &info));
  ASSERT_EQ(0, info.st_size);

  output.flush();
  EXPECT_EQ(kOk, output.status());
  ASSERT_EQ(0, ::fstat(::fileno(file), &info));
  EXPECT_EQ(1, info.st_size);
  output.close();
  EXPECT_EQ(kClosed, output.status());
}

// Verifies that ENOSPC from flushing reaches the typed writer before close.
TEST(PosixFileOutputStreamTest, FlushReportsNoSpaceBeforeClose) {
  std::FILE* file = std::fopen("/dev/full", "wb");
  ASSERT_NE(nullptr, file);
  auto output = std::make_unique<PosixFileOutputStream>(nullptr, file);
  ASSERT_EQ(0, std::setvbuf(file, nullptr, _IOFBF, BUFSIZ));

  OutputStreamWriter writer(std::move(output));
  writer.writeU8(0x11);
  ASSERT_EQ(kOk, writer.status());
  writer.flush();
  EXPECT_EQ(kNoSpaceLeftOnDevice, writer.status());
  writer.close();
  EXPECT_EQ(kNoSpaceLeftOnDevice, writer.status());
}

}  // namespace
}  // namespace roo_io
