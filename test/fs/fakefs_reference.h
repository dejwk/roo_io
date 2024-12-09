#pragma once

#include "fakefs.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {
namespace fakefs {

class FakeReferenceFs : public Filesystem {
 public:
  static constexpr bool strict = true;

  FakeReferenceFs(FakeFs& fs) : fs_(fs) {}

  MediaPresence checkMediaPresence() override { return kMediaPresent; }

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override {}

 private:
  FakeFs& fs_;
};

}  // namespace fakefs
}  // namespace roo_io