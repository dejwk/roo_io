#pragma once

#include <FS.h>

#include <memory>

#include "roo_io/fs/arduino/arduino_file_iterator.h"
#include "roo_io/resource/resource.h"

namespace roo_io {

class ArduinoFileIterable {
 public:
  ArduinoFileIterable(fs::FS& fs, String path)
      : fs_(fs), path_(std::move(path)) {}

  ArduinoFileIterator iterator() const {
    return ArduinoFileIterator(fs_.open(path_));
  }

 private:
  FS& fs_;
  String path_;
};

}  // namespace roo_io