#pragma once

#include <inttypes.h>

#include <memory>

#include "roo_io/stream/null_input_stream.h"
#include "roo_io/stream/null_output_stream.h"
#include "roo_io/stream/output_stream.h"
#include "roo_io/stream/random_access_input_stream.h"

namespace roo_io {

class FileImpl {
 public:
  ~FileImpl() = default;

  virtual bool isOpen() const = 0;
  virtual Status status() const = 0;
  virtual bool close() = 0;

  virtual bool isDirectory() const = 0;
  virtual uint64_t size() const = 0;
  virtual const char* path() const = 0;
  virtual const char* name() const = 0;

  virtual void rewindDirectory() = 0;
  virtual std::unique_ptr<FileImpl> openNextFile() = 0;

  virtual std::unique_ptr<RandomAccessInputStream> asInputStream() && = 0;
  virtual std::unique_ptr<OutputStream> asOutputStream() && = 0;

 protected:
  FileImpl() = default;
};

class File {
 public:
  File() = default;
  ~File() = default;
  File(File&& other) = default;

  File& operator=(File&& other) = default;

  bool isOpen() const;
  Status status() const;
  bool close();

  const char* path();
  const char* name();

  bool isDirectory() const;

  void rewindDirectory();

  File openNextFile();

  uint64_t size() const;

  std::unique_ptr<RandomAccessInputStream> asInputStream() &&;

  std::unique_ptr<OutputStream> asOutputStream() &&;

 private:
  friend class Mount;

  File(std::unique_ptr<FileImpl> file) : file_(std::move(file)) {}

  std::unique_ptr<FileImpl> file_;
};

}  // namespace roo_io