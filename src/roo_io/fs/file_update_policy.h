#pragma once

namespace roo_io {

// Passed to Mount::fopen and Mount::fopenForWrite, to specify what should
// happen when the target file already exists.
enum FileUpdatePolicy {
  kFailIfExists = 0,
  kTruncateIfExists = 1,
  kAppendIfExists = 2,
};

}  // namespace roo_io