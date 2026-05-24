#include <stdarg.h>
#include <stdio.h>

#include <string>

namespace roo_io {

/// Formats a string using `printf`-style arguments.
std::string StringPrintf(const char* format, ...);

/// Formats a string using a `va_list` argument pack.
std::string StringVPrintf(const char* format, va_list arg);

}  // namespace roo_io