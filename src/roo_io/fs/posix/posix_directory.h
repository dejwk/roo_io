#pragma once

#include <dirent.h>

#include <memory>
#include <string>

#include "roo_io/fs/directory.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

class PosixDirectoryImpl : public DirectoryImpl {
 public:
  PosixDirectoryImpl(std::shared_ptr<MountImpl> mount, const char* path,
                     DIR* dir, Status status);

  bool close() override;

  const char* path() const override { return path_.c_str(); }

  // const char* name() const override { return GetFileName(path_.c_str()); }

  Status status() const override { return status_; }

  void rewind() override;

  bool read(Directory::Entry& entry) override;

 private:
  std::shared_ptr<MountImpl> mount_;
  std::string path_;
  DIR* dir_;
  Status status_;
  struct dirent* next_;
  std::string file_;
};

}  // namespace roo_io