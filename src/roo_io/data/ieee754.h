#pragma once

// IEEE754 availability detection.
//
// ROO_IO_IEEE754 is enabled if:
//  1) IEEE754 conformance macros indicate support, and
//  2) float/double endianness appears compatible with integer endianness.
//
// Overrides:
//  - ROO_IO_FORCE_IEEE754: force enable regardless of auto-detection.
//  - ROO_IO_DISABLE_IEEE754: force disable regardless of auto-detection.

#if defined(ROO_IO_FORCE_IEEE754)
#define ROO_IO_IEEE754 1
#elif defined(ROO_IO_DISABLE_IEEE754)
#define ROO_IO_IEEE754 0
#else

#if defined(__STDC_IEC_559__) || defined(__STDCPP_IEC_559__) || \
        defined(__GCC_IEC_559)
#define ROO_IO_IEEE754_FORMAT 1
#else
#define ROO_IO_IEEE754_FORMAT 0
#endif

// Gate for float/integer endianness compatibility.
// If compiler exposes floating-point byte order, require it to match
// integer byte order. Otherwise, assume compatible by default.
#if defined(__FLOAT_WORD_ORDER__) && defined(__BYTE_ORDER__)
#define ROO_IO_IEEE754_ENDIAN_MATCH \
    (__FLOAT_WORD_ORDER__ == __BYTE_ORDER__)
#elif defined(__FLOAT_WORD_ORDER) && defined(__BYTE_ORDER)
#define ROO_IO_IEEE754_ENDIAN_MATCH (__FLOAT_WORD_ORDER == __BYTE_ORDER)
#else
#define ROO_IO_IEEE754_ENDIAN_MATCH 1
#endif

#if ROO_IO_IEEE754_FORMAT && ROO_IO_IEEE754_ENDIAN_MATCH
#define ROO_IO_IEEE754 1
#else
#define ROO_IO_IEEE754 0
#endif

#endif
