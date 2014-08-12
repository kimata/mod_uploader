/******************************************************************************
 * Copyright (C) 2007 Tetsuya Kimata <kimata@acapulco.dyndns.org>
 *
 * All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 *    not claim that you wrote the original software. If you use this
 *    software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must
 *    not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 * $Id: AtomicWrapper.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

/**
 * @file
 * @brief APR のアトミック関数の互換性をとるためのマクロ群．
 */

#ifndef ATOMIC_WRAPPER_H
#define ATOMIC_WRAPPER_H

#include "apr_atomic.h"
#include "apr_version.h"

#if APR_MAJOR_VERSION > 0
#define apr_atomic_t apr_uint32_t

#define apr_atomic_set      apr_atomic_set32
#define apr_atomic_inc      apr_atomic_inc32
#define apr_atomic_dec      apr_atomic_dec32
#define apr_atomic_read     apr_atomic_read32
#define apr_atomic_write    apr_atomic_write32
#define apr_atomic_cas      apr_atomic_cas32
#endif

// そのままでも十分早いのだけれど，どうせなら関数呼び出しを削除したい
#if defined(WIN32) && defined(_M_IX86)
#undef apr_atomic_read
#undef apr_atomic_cas
inline apr_uint32_t apr_atomic_read(volatile apr_atomic_t *memory)
{
    return *memory;
}
inline apr_uint32_t apr_atomic_cas(apr_atomic_t *memory, apr_uint32_t with,
                                     apr_uint32_t compare)
{
    apr_uint32_t result;

    __asm {
        mov eax, compare;
        mov ebx, memory;
        mov ecx, with;
        lock cmpxchg [ebx], ecx;
        mov result, eax;
    };

    return result;
}
#elif defined(GCC_ATOMIC_BUILTINS)
#undef apr_atomic_read
#undef apr_atomic_cas
inline apr_uint32_t apr_atomic_read(volatile apr_atomic_t *memory)
{
    return *memory;
}
inline apr_uint32_t apr_atomic_cas(apr_uint32_t *memory, apr_uint32_t with,
                                   apr_uint32_t compare)
{
    return __sync_val_compare_and_swap(memory, compare, with);
}
#elif defined(__linux__) && defined(__GNUC__) &&   \
    (defined(__i386__) || defined(__x86_64__))
#undef apr_atomic_read
#undef apr_atomic_cas
inline apr_uint32_t apr_atomic_read(volatile apr_atomic_t *memory)
{
    return *memory;
}
inline apr_uint32_t apr_atomic_cas(apr_uint32_t *memory, apr_uint32_t with,
                                   apr_uint32_t compare)
{
    apr_uint32_t result;

    asm volatile ("lock; cmpxchgl %1, %2"
                  : "=a" (result)
                  : "r" (with), "m" (*(memory)), "0"(compare)
                  : "memory", "cc");

    return result;
}
#endif

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
