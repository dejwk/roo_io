#pragma once

#include "fakefs.h"
#include "roo_io/fs/filesystem.h"

namespace roo_io {
namespace fakefs {

class FakeReferenceFs : public Filesystem {
 public:
  FakeReferenceFs() = default;

  MediaPresence checkMediaPresence() override { return kMediaPresent; }

 protected:
  MountImpl::MountResult mountImpl(std::function<void()> unmount_fn) override;

  void unmountImpl() override {}
};

}  // namespace fakefs
}  // namespace roo_io