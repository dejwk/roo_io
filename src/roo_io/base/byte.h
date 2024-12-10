#pragma once

#include <cstddef>

namespace roo_io {

#if __cplusplus >= 201703L
using byte = std::byte;
#else
using byte = unsigned char;
#endif

}