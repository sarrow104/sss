#ifndef  __ENDIAN_HPP_1449034417__
#define  __ENDIAN_HPP_1449034417__


//! http://esr.ibiblio.org/?p=5095

#include <stdint.h>

#define IS_BIG_ENDIAN (*(uint16_t *)"\0\xff" < 0x100)

//! http://stackoverflow.com/questions/2100331/c-macro-definition-to-determine-big-endian-or-little-endian-machine
//
// As @microtherion notes in comments, this relies on string literals being
// correct aligned for access as uint16_t, which is not guaranteed. If you have
// a compiler that supports C99 compound literals, you can avoid this problem:
//
//     #define IS_BIG_ENDIAN (!*(unsigned char *)&(uint16_t){1})
//
// or:
//
//     #define IS_BIG_ENDIAN (!(union { uint16_t u16; unsigned char c; }){ .u16 = 1 }.c)
//
// In general though, you should try to write code that does not depend on the
// endianness of the host platform.

#endif  /* __ENDIAN_HPP_1449034417__ */

