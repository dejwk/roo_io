#pragma once

#include <memory>
#include <string>

#include "roo_io/fs/filesystem.h"
#include "roo_io/resource/resource.h"

namespace roo_io {

class FileResource : public Resource {
 public:
  FileResource(Filesystem& fs, std::string path)
      : fs_(fs), path_(std::move(path)) {}

  std::unique_ptr<roo_io::InputStream> open() const override {
    return fs_.mount().openForReading(path_.c_str()).asInputStream();
  }

 private:
  Filesystem& fs_;
  std::string path_;
};

}  // namespace roo_io