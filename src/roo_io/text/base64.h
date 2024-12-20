#include "roo_io/base/byte.h"

namespace roo_io {

// Encodes the input as Base64. The size of the output must be at least ceil(4 *
// input_length / 3).
void Base64Encode(const byte* input, size_t input_length, char* output);

// void Base64Decode(const char* input, size_t input_length, byte* output);

}  // namespace roo_io