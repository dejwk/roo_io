#include <cstddef>

#include "roo_io/base/byte.h"

namespace roo_io {

/// Encodes `input` as Base64 into `output` without appending a terminator.
///
/// The caller must provide at least `ceil(4 * input_length / 3)` bytes of
/// storage in `output`.
void Base64Encode(const byte* input, size_t input_length, char* output);

// void Base64Decode(const char* input, size_t input_length, byte* output);

}  // namespace roo_io