#include "roo_io/text/string_printf.h"

#include <memory>

namespace roo_io {

std::string StringPrintf(const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  std::string result = StringVPrintf(format, arg);
  va_end(arg);
  return result;
}

std::string StringVPrintf(const char* format, va_list arg) {
  // Opportunistically try with a small buffer, to minimize the risk of
  // overflowing the stack, but also, avoid dynamic memory allocation in the
  // common case.
  char buf[128];
  int len = vsnprintf(buf, 128, format, arg);

  if (len <= 127) {
    return std::string(buf, len);
  }
  std::unique_ptr<char[]> dbuf(new char[len + 1]);
  vsnprintf(dbuf.get(), len + 1, format, arg);
  return std::string(dbuf.get(), len);
}

}  // namespace roo_io