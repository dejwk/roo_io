#pragma once

#include <FS.h>

#include <memory>

#include "roo_io/fs/arduino/arduino_file_input_stream.h"
#include "roo_io/resource/resource.h"

namespace roo_io {

class ArduinoFileResource : public Resource {
 public:
  ArduinoFileResource(fs::FS& fs, String path) : fs_(fs), path_(std::move(path)) {}

  std::unique_ptr<RandomAccessInputStream> open() const override {
    return std::unique_ptr<RandomAccessInputStream>(
        new ArduinoFileInputStream(fs_.open(path_)));
  }

 private:
  FS& fs_;
  String path_;
};

}  // namespace roo_io