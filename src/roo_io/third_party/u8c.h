/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
**
** This software is distributed under the terms of the MIT license:
**  https://opensource.org/licenses/MIT
**
**  *** UTF-8 encode/decode ***
**
**  int u8next(char *s [, int *c]) --> Returns the number of bytes encoding the first
**                                     codepoint in the string s.
**                                     If c is not NULL, stores the codepoint in *c.
**                                     If the encoding is not valid, returns -1 and stores
**                                     in *c the first byte of the string.
**                                     Note that c is optional (implies NULL).
**
**  int u8strlen(char *s)          --> Returns the number of codepoints in the string s.
**                                     Does NOT validate the string encoding!
**
**  int u8codepoint(char *s)       --> Returns the codepoint encoded in s or the first
**                                     byte if the encodinging is not valid.
**
**  int u8encode(int c[, char *s]) --> Stores the encoding of codepoint c in s and
**                                     place a string terminator ('\0') at the end.
**                                     There must be *at least* 5 bytes allocated in the
**                                     string s. s can be NULL (or omitted entirely).
**                                     Returns the length of the encoding in bytes. 
**                                     
**  
**  The decoding function is based on the work of Bjoern Hoehrmann:
**     http://bjoern.hoehrmann.de/utf-8/decoder/dfa
**
**  with the following differences:
**    - the implementation is faster than what offered on the original site.
**    - the code is clearer to read and to relate to Bjoern's state machines
**    - it has been extended to include C0 80 as the encoding for U+0000.
**      (see https://en.wikipedia.org/wiki/UTF-8#Modified_UTF-8 )
**
**  To use it, include u8c.h in your code and link against u8c.c
*/

#ifndef U8C_H__
#define U8C_H__

#include <string.h>

// #define u8_exp(x) x
// #define u8_1(x,...)     x
// #define u8_2(x,y,...)   y

namespace u8c {

// size_t u8strlen(const char *s);
size_t u8next_(const char *start, const char* end, char32_t& ch);

}  // namespace u8c

#endif