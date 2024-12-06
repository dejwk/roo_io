#pragma once

#include <memory>

#include "FS.h"
#include "roo_io/fs/arduino/arduino_file_input_stream.h"
#include "roo_io/fs/arduino/arduino_file_output_stream.h"
#include "roo_io/fs/file.h"
#include "roo_io/stream/multipass_input_stream.h"
#include "roo_io/stream/null_input_stream.h"
#include "roo_io/stream/output_stream.h"

namespace roo_io {

// class ArduinoFileImpl : public FileImpl {
//  public:
//   ArduinoFileImpl(fs::File file, Status status)
//       : file_(std::move(file)), status_(status) {}

//   bool isOpen() const override { return file_.operator bool(); }

//   bool close() override {
//     file_.close();
//     if (status_ = kOk) status_ = kClosed;
//     return true;
//   }

//   Status status() const override { return status_; }

//   uint64_t size() const override { return file_.size(); }

//   const char* path() const override { return file_.path(); }

//   const char* name() const override { return file_.name(); }

//   std::unique_ptr<MultipassInputStream> asInputStream() && override {
//     return std::unique_ptr<MultipassInputStream>(
//         status_ == kOk ? (MultipassInputStream*)new ArduinoFileInputStream(
//                              std::move(file_))
//                        : (MultipassInputStream*)new NullInputStream(status_));
//   }

//   std::unique_ptr<OutputStream> asOutputStream() && override {
//     return std::unique_ptr<OutputStream>(
//         status_ == kOk
//             ? (OutputStream*)new ArduinoFileOutputStream(std::move(file_))
//             : (OutputStream*)new NullOutputStream(status_));
//   }

//  private:
//   mutable fs::File file_;
//   Status status_;
// };

}  // namespace roo_io