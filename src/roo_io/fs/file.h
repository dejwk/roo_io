// #pragma once

// #include <inttypes.h>

// #include <memory>

// #include "roo_io/stream/multipass_input_stream.h"
// #include "roo_io/stream/null_input_stream.h"
// #include "roo_io/stream/null_output_stream.h"
// #include "roo_io/stream/output_stream.h"

// namespace roo_io {

// class FileImpl {
//  public:
//   ~FileImpl() = default;

//   virtual bool isOpen() const = 0;
//   virtual Status status() const = 0;
//   virtual bool close() = 0;

//   virtual uint64_t size() const = 0;
//   virtual const char* path() const = 0;
//   virtual const char* name() const = 0;

//   virtual std::unique_ptr<MultipassInputStream> asInputStream() && = 0;
//   virtual std::unique_ptr<OutputStream> asOutputStream() && = 0;

//  protected:
//   FileImpl() = default;
// };

// class File {
//  public:
//   File(Status status = kClosed) : status_(status) {}

//   ~File() = default;
//   File(File&& other) = default;

//   File& operator=(File&& other) = default;

//   bool ok() const { return status() == kOk; }

//   bool isOpen() const;
//   Status status() const;
//   bool close();

//   const char* path() const;
//   const char* name() const;

//   uint64_t size() const;

//   std::unique_ptr<MultipassInputStream> asInputStream() &&;

//   std::unique_ptr<OutputStream> asOutputStream() &&;

//  private:
//   friend class Mount;

//   File(std::unique_ptr<FileImpl> file)
//       : file_(std::move(file)),
//         status_(file_ == nullptr ? kClosed : file_->status()) {}

//   std::unique_ptr<FileImpl> file_;
//   Status status_;
// };

// }  // namespace roo_io