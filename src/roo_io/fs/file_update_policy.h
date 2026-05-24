#pragma once

namespace roo_io {

/// Describes how file-creation helpers behave when the target already exists.
enum FileUpdatePolicy {
  kFailIfExists = 0,
  kTruncateIfExists = 1,
  kAppendIfExists = 2,
};

}  // namespace roo_io