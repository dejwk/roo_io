#include "fakefs_reference.h"

namespace roo_io {
namespace fakefs {

class FakeDirectory : public DirectoryImpl {
 public:
  FakeDirectory(std::string path, DirIterator itr)
      : path_(std::move(path)), itr_(std::move(itr)) {}

  const char* path() const override { return path_.c_str(); }

  const char* name() const override { return GetFileName(path_.c_str()); }

  Status status() const override {
    return itr_.isOpen() ? (itr_.ok() ? kOk : kEndOfStream) : kClosed;
  }

  bool close() override {
    itr_.close();
    return true;
  }

  void rewind() override { itr_.rewind(); }

  Entry read() override {
    itr_.next();
    if (!itr_.ok()) {
      return Entry();
    }
    entry_path_ = path_ + "/" + itr_.entry().name();
    return CreateEntry(entry_path_.c_str(), path_.size() + 1,
                       itr_.entry().isDir());
  }

 private:
  std::string path_;
  DirIterator itr_;
  std::string entry_path_;
};

class FakeInputStream : public MultipassInputStream {
 public:
  FakeInputStream(FileStream f) : f_(std::move(f)) {}

  bool isOpen() const override { return f_.isOpen(); }

  void close() override { f_.close(); }

  size_t read(byte* result, size_t count) override {
    return f_.read(result, count);
  }

  void skip(uint64_t count) override { f_.skip(count); }

  Status status() const override { return f_.status(); }

  uint64_t size() override { return f_.size(); }

  uint64_t position() const override { return f_.position(); }

  void seek(uint64_t position) override { f_.seek(position); }

 private:
  FileStream f_;
};

class FakeOutputStream : public OutputStream {
 public:
  FakeOutputStream(FileStream f) : f_(std::move(f)) {}

  bool isOpen() const override { return f_.isOpen(); }

  size_t write(const byte* buf, size_t count) override {
    return f_.write(buf, count);
  }

  Status status() const override { return f_.status(); }

 private:
  FileStream f_;
};

class FakeMount : public MountImpl {
 public:
  FakeMount(FakeFs& fs, bool read_only, std::function<void()> unmount_fn)
      : MountImpl(unmount_fn), fs_(fs), read_only_(read_only) {}

  bool isReadOnly() const override { return read_only_; }

  Stat stat(const char* path) const override {
    StatResult s = fs_.stat(path);
    if (s.status != kOk) return Stat(s.status);
    if (s.type == StatResult::kDir) {
      return Stat(Stat::kDir, 0);
    } else {
      return Stat(Stat::kFile, s.size);
    }
  }

  Status remove(const char* path) override { return fs_.remove(path); }

  Status rename(const char* pathFrom, const char* pathTo) override {
    return fs_.rename(pathFrom, pathTo);
  }

  Status mkdir(const char* path) override { return fs_.mkdir(path); }

  Status rmdir(const char* path) override { return fs_.rmdir(path); }

  std::unique_ptr<DirectoryImpl> opendir(const char* path) override {
    DirIterator itr;
    Status status = fs_.opendir(path, itr);
    if (status != kOk) return DirectoryError(status);
    return std::unique_ptr<DirectoryImpl>(
        new FakeDirectory(path, std::move(itr)));
  }

  std::unique_ptr<MultipassInputStream> fopen(const char* path) override {
    FileStream f = fs_.open(path, FakeFs::kRead);
    if (!f.isOpen()) {
      return InputError(f.status());
    }
    return std::unique_ptr<MultipassInputStream>(
        new FakeInputStream(std::move(f)));
  }

  std::unique_ptr<OutputStream> fopenForWrite(
      const char* path, FileUpdatePolicy update_policy) override {
    int flags = FakeFs::kWrite;
    switch (update_policy) {
      case kAppendIfExists: {
        flags |= FakeFs::kAppend;
        break;
      }
      case kTruncateIfExists: {
        flags |= FakeFs::kTruncate;
        break;
      }
      default: {
      }
    }
    FileStream f = fs_.open(path, flags);
    if (!f.isOpen()) {
      return OutputError(f.status());
    }
    return std::unique_ptr<OutputStream>(new FakeOutputStream(std::move(f)));
  }

 private:
  FakeFs& fs_;
  bool read_only_;
};

MountImpl::MountResult FakeReferenceFs::mountImpl(
    std::function<void()> unmount_fn) {
  return MountImpl::Mounted(
      std::unique_ptr<MountImpl>(new FakeMount(fs_, false, unmount_fn)));
}

}  // namespace fakefs
}  // namespace roo_io