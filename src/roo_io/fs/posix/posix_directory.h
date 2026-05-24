#pragma once

#include "roo_io/fs/posix/config.h"

#if ROO_IO_FS_SUPPORT_POSIX

#include <dirent.h>

#include <memory>
#include <string>

#include "roo_io/fs/directory.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {

/// POSIX directory iterator implementation.
class PosixDirectoryImpl : public DirectoryImpl {
 public:
  /// Opens or adopts a POSIX directory handle rooted at `path`.
  PosixDirectoryImpl(std::shared_ptr<MountImpl> mount, const char* path,
                     DIR* dir, Status status);

  /// Closes the underlying directory handle.
  bool close() override;

  /// Returns the absolute directory path.
  const char* path() const override { return path_.c_str(); }

  // const char* name() const override { return GetFileName(path_.c_str()); }

  /// Returns the current directory status.
  Status status() const override { return status_; }

  /// Rewinds iteration to the first entry.
  void rewind() override;

  /// Reads the next directory entry into `entry`.
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

#endif  // ROO_IO_FS_SUPPORT_POSIX
