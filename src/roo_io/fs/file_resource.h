#pragma once

#include <memory>
#include <string>

#include "roo_io/core/resource.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {
/// Multipass resource that reopens the same filesystem path on demand.
class FileResource : public MultipassResource {
 public:
  /// Stores the filesystem and path used by future `open()` calls.
  FileResource(Filesystem& fs, std::string path)
      : fs_(fs), path_(std::move(path)) {}

  /// Opens the stored path for reading through a new mount.
  std::unique_ptr<roo_io::MultipassInputStream> open() const override {
    return fs_.mount().fopen(path_.c_str());
  }

 private:
  Filesystem& fs_;
  std::string path_;
};

}  // namespace roo_io