#include "roo_io/text/base64.h"

#include "roo_io/third_party/base64/arduino_base64.h"

namespace roo_io {

void Base64Encode(const byte* input, size_t input_length, char* output) {
  base64::encode((const uint8_t*)input, input_length, output);
}

}  // namespace roo_io