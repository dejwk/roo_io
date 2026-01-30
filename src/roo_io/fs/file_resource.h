#pragma once

#include <memory>
#include <string>

#include "roo_io/core/resource.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {
class FileResource : public MultipassResource {
 public:
  FileResource(Filesystem& fs, std::string path)
      : fs_(fs), path_(std::move(path)) {}

  std::unique_ptr<roo_io::MultipassInputStream> open() const override {
    return fs_.mount().fopen(path_.c_str());
  }

 private:
  Filesystem& fs_;
  std::string path_;
};

}  // namespace roo_io