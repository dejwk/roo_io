#include "fakefs.h"

namespace roo_io {
namespace fakefs {

size_t File::read(size_t pos, byte* buf, size_t size) const {
  if (pos > data_.size()) return 0;
  if (pos + size > data_.size()) size = data_.size() - pos;
  memcpy(buf, &data_[pos], size);
  return size;
}

size_t File::write(size_t pos, const byte* buf, size_t size) {
  if (data_.size() < pos + size) {
    data_.resize(pos + size);
  }
  memcpy(&data_[pos], buf, size);
  return size;
}

Entry* Dir::find(const std::string& name) {
  std::list<std::unique_ptr<Entry>>::iterator itr = lookup(name);
  if (itr == entries_.end()) return nullptr;
  return itr->get();
}

void Dir::rename(const std::string& oldname, const std::string& newname) {
  Entry* oldentry = find(oldname);
  CHECK(oldentry != nullptr);
  Entry* newentry = find(newname);
  CHECK(newentry == nullptr);
  oldentry->rename(newname);
}

roo_io::Status Dir::mkdir(const Entry* parent, const std::string& name,
                          Entry** dir) {
  if (dir != nullptr) *dir = nullptr;
  std::list<std::unique_ptr<Entry>>::iterator itr = lookup(name);
  if (itr != entries_.end()) {
    if ((*itr)->isDir()) {
      return kDirectoryExists;
    } else {
      return kFileExists;
    }
  }
  entries_.emplace_back(Entry::DirEntry(name));
  entries_.back()->setParent(parent);
  if (dir != nullptr) *dir = entries_.back().get();
  return kOk;
}

Status Dir::create(const Entry* parent, const std::string& name, Entry** file) {
  if (file != nullptr) *file = nullptr;
  std::list<std::unique_ptr<Entry>>::iterator itr = lookup(name);
  if (itr != entries_.end()) {
    if ((*itr)->isDir()) {
      return kDirectoryExists;
    } else {
      return kFileExists;
    }
  }
  entries_.emplace_back(Entry::FileEntry(name));
  entries_.back()->setParent(parent);
  if (file != nullptr) *file = entries_.back().get();
  return kOk;
}

roo_io::Status Dir::rmdir(const std::string& name) {
  std::list<std::unique_ptr<Entry>>::iterator itr = lookup(name);
  if (itr == entries_.end()) {
    return kNotFound;
  }
  Entry& e = **itr;
  if (!e.isDir()) {
    return kNotDirectory;
  }
  if (e.dir().entryCount() != 0) {
    return kDirectoryNotEmpty;
  }
  entries_.erase(itr);
  return kOk;
}

roo_io::Status Dir::rm(const std::string& name) {
  std::list<std::unique_ptr<Entry>>::iterator itr = lookup(name);
  if (itr == entries_.end()) {
    return kNotFound;
  }
  Entry& e = **itr;
  if (!e.isFile()) {
    return kNotFile;
  }
  entries_.erase(itr);
  return kOk;
}

std::unique_ptr<Entry> Dir::detach(const std::string& name) {
  for (std::list<std::unique_ptr<Entry>>::iterator i = entries_.begin();
       i != entries_.end(); ++i) {
    if ((*i)->name() == name) {
      std::unique_ptr<Entry> result = std::move(*i);
      entries_.erase(i);
      result->setParent(nullptr);
      return result;
    }
  }
  return nullptr;
}

void Dir::attach(const Entry* parent, const std::string& name,
                 std::unique_ptr<Entry> entry) {
  CHECK(lookup(name) == entries_.end());
  entry->rename(name);
  entry->setParent(parent);
  entries_.push_back(std::move(entry));
}

std::list<std::unique_ptr<Entry>>::iterator Dir::lookup(
    const std::string& name) {
  for (std::list<std::unique_ptr<Entry>>::iterator i = entries_.begin();
       i != entries_.end(); ++i) {
    if ((*i)->name() == name) return i;
  }
  return entries_.end();
}

void DirIterator::open(Dir& dir) {
  // status_ = kOk;
  dir_ = &dir;
  current_ = {};
  bos_ = true;
}

void DirIterator::close() {
  dir_ = nullptr;
  current_ = {};
  bos_ = true;
}

bool DirIterator::isOpen() const { return dir_ != nullptr; }

bool DirIterator::ok() const {
  return isOpen() && current_ != dir_->entries_.end();
}

void DirIterator::rewind() {
  if (dir_ != nullptr) {
    current_ = {};
    bos_ = true;
  }
}

bool DirIterator::next() {
  if (!isOpen()) return false;
  if (bos_) {
    current_ = dir_->entries_.begin();
    bos_ = false;
    return (current_ != dir_->entries_.end());
  } else {
    if (current_ == dir_->entries_.end()) return false;
    ++current_;
    return true;
  }
}

std::unique_ptr<Entry> Entry::DirEntry(const std::string& name) {
  return std::unique_ptr<Entry>(
      new Entry(name, std::unique_ptr<Dir>(new Dir())));
}

std::unique_ptr<Entry> Entry::FileEntry(const std::string& name) {
  return std::unique_ptr<Entry>(
      new Entry(name, std::unique_ptr<File>(new File())));
}

bool Entry::isDescendantOf(const Entry& e) const {
  if (parent() == nullptr) {
    // I descent from nobody.
    return false;
  }
  if (parent() == &e) return true;
  return parent()->isDescendantOf(e);
}

FileStream::FileStream(Status status)
    : file_(nullptr), read_only_(false), position_(0), status_(status) {}

void FileStream::setError(Status status) {
  file_ = nullptr;
  read_only_ = false;
  position_ = 0;
  status_ = status;
}

void FileStream::open(File* file, bool readonly) {
  file_ = file;
  position_ = 0;
  read_only_ = readonly;
  status_ = kOk;
}

void FileStream::close() {
  file_ = nullptr;
  status_ = kClosed;
}

size_t FileStream::read(byte* target, size_t size) {
  if (!isOpen()) return 0;
  if (status_ != kOk) return 0;
  size_t result = file_->read(position_, target, size);
  if (result < size) {
    status_ = kEndOfStream;
  }
  return result;
}

size_t FileStream::write(const byte* source, size_t size) {
  if (!isOpen()) return 0;

  if (read_only_) {
    status_ = kReadOnlyFilesystem;
    return 0;
  }
  return file_->write(position_, source, size);
}

void FileStream::seek(size_t pos) {
  if (status_ != kOk && status_ != kEndOfStream) return;
  position_ = pos;
  status_ = kOk;
}

StatResult FakeFs::stat(const char* path) const {
  Entry* entry;
  Status status = findEntryByPath(path, &entry, false);
  if (status != kOk) {
    return StatResult{
        .status = status, .type = StatResult::kUnknown, .size = 0};
  }
  if (entry->isDir()) {
    return StatResult{.status = kOk, .type = StatResult::kDir, .size = 0};
  } else {
    return StatResult{
        .status = kOk, .type = StatResult::kFile, entry->file().size()};
  }
}

Status FakeFs::remove(const char* path) {
  ResolvedPath resolved = resolvePath(path);
  if (resolved.status != kOk) return resolved.status;
  if (resolved.parent == nullptr) return kInvalidPath;
  return resolved.parent->dir().rm(resolved.basename);
}

Status FakeFs::rename(const char* pathFrom, const char* pathTo) {
  ResolvedPath resolvedFrom = resolvePath(pathFrom);
  if (resolvedFrom.status != kOk) return resolvedFrom.status;
  if (resolvedFrom.parent == nullptr) return kInvalidPath;
  ResolvedPath resolvedTo = resolvePath(pathTo);
  if (resolvedTo.status != kOk) return resolvedTo.status;
  if (resolvedTo.parent == nullptr) return kInvalidPath;
  Entry* src = resolvedFrom.parent->dir().find(resolvedFrom.basename);
  if (src == nullptr) return kNotFound;

  Entry* conflicting = resolvedTo.parent->dir().find(resolvedTo.basename);
  if (conflicting != nullptr) {
    if (conflicting->isDir()) {
      return kDirectoryExists;
    } else {
      return kFileExists;
    }
  }
  if (resolvedTo.parent == resolvedFrom.parent) {
    resolvedFrom.parent->dir().rename(resolvedFrom.basename,
                                      resolvedTo.basename);
    return kOk;
  }
  if (resolvedTo.parent->isDescendantOf(*src)) {
    return kInvalidPath;
  }

  resolvedTo.parent->dir().attach(
      resolvedTo.parent, resolvedTo.basename,
      resolvedFrom.parent->dir().detach(resolvedFrom.basename));
  return kOk;
}

Status FakeFs::mkdir(const char* path) {
  ResolvedPath resolved = resolvePath(path);
  if (resolved.status != kOk) return resolved.status;
  if (resolved.parent == nullptr) return kInvalidPath;

  return resolved.parent->dir().mkdir(resolved.parent, resolved.basename,
                                      nullptr);
}

Status FakeFs::rmdir(const char* path) {
  ResolvedPath resolved = resolvePath(path);
  if (resolved.status != kOk) return resolved.status;
  if (resolved.parent == nullptr) return kInvalidPath;
  return resolved.parent->dir().rmdir(resolved.basename);
}

Status FakeFs::opendir(const char* path, DirIterator& itr) {
  Entry* entry;
  Status status = findEntryByPath(path, &entry, false);
  if (status != kOk) {
    return status;
  }
  if (!entry->isDir()) {
    return kNotDirectory;
  }
  itr.open(entry->dir());
  return kOk;
}

FileStream FakeFs::open(const char* path, int flags) {
  ResolvedPath resolved = resolvePath(path);
  if (resolved.status != kOk) {
    return FileStream(resolved.status);
  }
  if (resolved.parent == nullptr) {
    return FileStream(kInvalidPath);
  }
  bool read_only = (flags & (kWrite | kAppend)) == 0;
  Entry* file = resolved.parent->dir().find(resolved.basename);
  if (file == nullptr) {
    if (read_only || (flags & kTruncate) == 0) {
      return FileStream(kNotFound);
    }
    Status status = resolved.parent->dir().create(resolved.parent,
                                                  resolved.basename, &file);
    if (status != kOk) return FileStream(status);
  } else {
    if (!file->isFile()) {
      return FileStream(kNotFile);
    }
  }
  FileStream result;
  result.open(&file->file(), read_only);
  if ((flags & kAppend) != 0) {
    result.seek(result.size());
  }
  return result;
}

Status FakeFs::findEntryByPath(const char* name, Entry** out,
                               bool create_subdirs) const {
  if (out != nullptr) *out = nullptr;
  if (name[0] != '/') return kInvalidPath;
  Entry* dir = root_.get();
  const char* p = name + 1;
  while (true) {
    while (*p == '/') ++p;
    if (*p == 0) {
      *out = dir;
      return kOk;
    }
    const char* e = p;
    while (*e != 0 && *e != '/') ++e;
    bool subdir = (*e == '/');
    std::string name(p, e - p);
    Entry* entry = dir->dir().find(name);
    if (entry == nullptr) {
      if (create_subdirs) {
        Status s = dir->dir().mkdir(dir, name, &entry);
        if (s != kOk) return s;
      } else {
        return kNotFound;
      }
    }
    if (subdir) {
      if (!entry->isDir()) {
        return kNotDirectory;
      }
      dir = entry;
      p = e + 1;
      continue;
    }
    if (out != nullptr) *out = entry;
    return kOk;
  }
}

ResolvedPath FakeFs::resolvePath(const char* path, bool create_subdirs) {
  std::string s(path);
  if (s.empty() || s[0] != '/') {
    return ResolvedPath{.status = kInvalidPath};
  }
  while (!s.empty() && s.back() == '/') s.pop_back();
  if (s.empty()) {
    // Root directory: OK, but no parent, and empty name.
    return ResolvedPath{.status = kOk, .parent = nullptr, .basename = ""};
  }
  auto pos = s.find_last_of("/");
  CHECK(pos != std::string::npos);
  ResolvedPath result;
  result.basename = s.substr(pos + 1);
  if (pos > 0) {
    Status status = findEntryByPath(s.substr(0, pos).c_str(), &result.parent,
                                    create_subdirs);
    result.status = status != kOk              ? status
                    : (result.parent->isDir()) ? kOk
                                               : kNotDirectory;
  } else {
    result.status = kOk;
    result.parent = root_.get();
  }
  return result;
}

Status CreateTextFile(FakeFs& fs, const char* path, const char* contents) {
  ResolvedPath resolved = fs.resolvePath(path, true);
  if (resolved.status != kOk) return resolved.status;
  Entry* file;
  Status creation_status =
      resolved.parent->dir().create(resolved.parent, resolved.basename, &file);
  if (creation_status != kOk) return creation_status;
  file->file().write(0, (const byte*)contents, strlen(contents));
  return kOk;
}

std::string ReadTextFile(FakeFs& fs, const char* path) {
  ResolvedPath resolved = fs.resolvePath(path, true);
  if (resolved.status != kOk) return "";
  Entry* existing = resolved.parent->dir().find(resolved.basename);
  if (existing == nullptr) return "";
  if (!existing->isFile()) return "";
  size_t size = existing->file().size();
  std::unique_ptr<char[]> result(new char[size]);
  existing->file().read(0, (byte*)result.get(), size);
  return std::string(result.get(), size);
}

}  // namespace fakefs
}  // namespace roo_io
