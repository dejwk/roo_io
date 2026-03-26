#pragma once

// Utility methods for fast-comparing memory regions against specified bit
// pattern fills. The methods optimize performance by reducing the frequency of
// branching and reading large data blocks when possible.

#include <inttypes.h>

#include <cstring>

#include "roo_io/base/byte.h"

namespace roo_io {

template <int bytes>
inline bool PatternCompare(const byte* buf, size_t count, const byte* val);

template <int bytes>
inline bool PatternCompareAligned(const byte* buf, size_t count,
                                  const byte* val);

template <>
inline bool PatternCompare<1>(const byte* buf, size_t count, const byte* val) {
  while (count >= 4) {
    if (buf[0] != *val || buf[1] != *val || buf[2] != *val || buf[3] != *val) {
      return false;
    }
    buf += 4;
    count -= 4;
  }
  while (count-- > 0) {
    if (*buf++ != *val) return false;
  }
  return true;
}

template <>
inline bool PatternCompareAligned<1>(const byte* buf, size_t count,
                                     const byte* val) {
  return PatternCompare<1>(buf, count, val);
}

template <>
inline __attribute__((always_inline)) bool PatternCompare<2>(const byte* buf,
                                                             size_t count,
                                                             const byte* val) {
  if (((intptr_t)buf & 1) != 0) {
    // mis-aligned.
    const roo::byte v0 = val[0];
    const roo::byte v1 = val[1];
    while (count >= 4) {
      if (buf[0] != v0 || buf[1] != v1 || buf[2] != v0 || buf[3] != v1 ||
          buf[4] != v0 || buf[5] != v1 || buf[6] != v0 || buf[7] != v1) {
        return false;
      }
      buf += 8;
      count -= 4;
    }
    while (count-- > 0) {
      if (buf[0] != v0 || buf[1] != v1) return false;
      buf += 2;
    }
    return true;
  } else {
    // aligned.
    uint16_t v;
    ((byte*)&v)[0] = val[0];
    ((byte*)&v)[1] = val[1];
    const uint16_t* buf16 = (const uint16_t*)buf;
    while (count >= 4) {
      if (buf16[0] != v || buf16[1] != v || buf16[2] != v || buf16[3] != v) {
        return false;
      }
      buf16 += 4;
      count -= 4;
    }
    while (count-- > 0) {
      if (*buf16++ != v) return false;
    }
    return true;
  }
}

template <>
inline __attribute__((always_inline)) bool PatternCompareAligned<2>(
    const byte* buf, size_t count, const byte* val) {
  uint16_t v = *(const uint16_t*)val;
  const uint16_t* buf16 = (const uint16_t*)buf;
  while (count >= 4) {
    if (buf16[0] != v || buf16[1] != v || buf16[2] != v || buf16[3] != v) {
      return false;
    }
    buf16 += 4;
    count -= 4;
  }
  while (count-- > 0) {
    if (*buf16++ != v) return false;
  }
  return true;
}

template <>
inline __attribute__((always_inline)) bool PatternCompare<3>(const byte* buf,
                                                             size_t count,
                                                             const byte* val) {
  while (count >= 4) {
    if (buf[0] != val[0] || buf[1] != val[1] || buf[2] != val[2] ||
        buf[3] != val[0] || buf[4] != val[1] || buf[5] != val[2] ||
        buf[6] != val[0] || buf[7] != val[1] || buf[8] != val[2] ||
        buf[9] != val[0] || buf[10] != val[1] || buf[11] != val[2]) {
      return false;
    }
    buf += 12;
    count -= 4;
  }
  while (count-- > 0) {
    if (buf[0] != val[0] || buf[1] != val[1] || buf[2] != val[2]) return false;
    buf += 3;
  }
  return true;
}

template <>
inline __attribute__((always_inline)) bool PatternCompareAligned<3>(
    const byte* buf, size_t count, const byte* val) {
  return PatternCompare<3>(buf, count, val);
}

template <>
inline __attribute__((always_inline)) bool PatternCompare<4>(const byte* buf,
                                                             size_t count,
                                                             const byte* val) {
  if (((intptr_t)buf & 3) != 0) {
    while (count >= 4) {
      if (buf[0] != val[0] || buf[1] != val[1] || buf[2] != val[2] ||
          buf[3] != val[3] || buf[4] != val[0] || buf[5] != val[1] ||
          buf[6] != val[2] || buf[7] != val[3] || buf[8] != val[0] ||
          buf[9] != val[1] || buf[10] != val[2] || buf[11] != val[3] ||
          buf[12] != val[0] || buf[13] != val[1] || buf[14] != val[2] ||
          buf[15] != val[3]) {
        return false;
      }
      buf += 16;
      count -= 4;
    }
    while (count-- > 0) {
      if (buf[0] != val[0] || buf[1] != val[1] || buf[2] != val[2] ||
          buf[3] != val[3])
        return false;
      buf += 4;
    }
    return true;
  } else {
    uint32_t v;
    ((byte*)&v)[0] = val[0];
    ((byte*)&v)[1] = val[1];
    ((byte*)&v)[2] = val[2];
    ((byte*)&v)[3] = val[3];
    const uint32_t* buf32 = (const uint32_t*)buf;
    while (count >= 4) {
      if (buf32[0] != v || buf32[1] != v || buf32[2] != v || buf32[3] != v) {
        return false;
      }
      buf32 += 4;
      count -= 4;
    }
    while (count-- > 0) {
      if (*buf32++ != v) return false;
    }
    return true;
  }
}

template <>
inline __attribute__((always_inline)) bool PatternCompareAligned<4>(
    const byte* buf, size_t count, const byte* val) {
  const uint32_t v = (*(const uint32_t*)val);
  const uint32_t* buf32 = (const uint32_t*)buf;
  while (count >= 4) {
    if (buf32[0] != v || buf32[1] != v || buf32[2] != v || buf32[3] != v) {
      return false;
    }
    buf32 += 4;
    count -= 4;
  }
  while (count-- > 0) {
    if (*buf32++ != v) return false;
  }
  return true;
}

}  // namespace roo_io