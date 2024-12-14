#include "roo_io/fs/mount_impl.h"

#include "roo_io/stream/null_input_stream.h"
#include "roo_io/stream/null_output_stream.h"

namespace roo_io {

MountImpl::MountResult MountImpl::Mounted(
    std::unique_ptr<MountImpl> mount_impl) {
  return MountImpl::MountResult{.status = kOk, .mount = std::move(mount_impl)};
}

MountImpl::MountResult MountImpl::MountError(Status status) {
  return MountImpl::MountResult{.status = status, .mount = nullptr};
}

namespace {

class DirectoryErrorImpl : public DirectoryImpl {
 public:
  DirectoryErrorImpl(Status status) : status_(status) {}

  const char* path() const override { return nullptr; }
  // const char* name() const override { return nullptr; }

  Status status() const override { return status_; }
  bool close() override { return true; }
  void rewind() override {}
  bool read(Directory::Entry&) override { return false; }

 private:
  Status status_;
};

}  // namespace

std::unique_ptr<DirectoryImpl> DirectoryError(Status error) {
  return std::unique_ptr<DirectoryImpl>(new DirectoryErrorImpl(error));
}

std::unique_ptr<MultipassInputStream> InputError(Status error) {
  return std::unique_ptr<MultipassInputStream>(new NullInputStream(error));
}

std::unique_ptr<OutputStream> OutputError(Status error) {
  return std::unique_ptr<OutputStream>(new NullOutputStream(error));
}

}  // namespace roo_io