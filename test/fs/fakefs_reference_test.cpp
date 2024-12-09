#include "fakefs_reference.h"

#include "fs_p.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace roo_io {

INSTANTIATE_TYPED_TEST_SUITE_P(ReferenceFsTest, FsTest, fakefs::FakeReferenceFs);

}  // namespace roo_io
