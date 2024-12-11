#include <stdarg.h>
#include <stdio.h>

#include <string>

namespace roo_io {

// Like printf, but outputs to a string.
std::string StringPrintf(const char* format, ...);

// Like vsnprintf, but outputs to a C++ string.
std::string StringVPrintf(const char* format, va_list arg);

}