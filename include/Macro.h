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
 * $Id: Macro.h 2873 2008-04-27 14:19:47Z svn $
 *****************************************************************************/

/**
 * @file
 * @brief 雑多なマクロ群．
 */

#ifndef MACRO_H
#define MACRO_H

#include "Environment.h"

#include <cstdlib>
#include <iostream>
#ifdef DEBUG
#include <iomanip>
#endif

#include "apr_time.h"

#include "Message.h"

/**
 * 内容にファイル名と行番号を追加して例外を投げます．
 *
 * @param[in] str 内容
 */
#ifdef DEBUG
#define THROW(str) throw str " (" __FILE__ ":"  \
    APR_STRINGIFY(__LINE__) ")"
#else
#define THROW(str) throw str
#endif
/**
 * メモリバリアです．
 */
#if defined(GCC_ATOMIC_BUILTINS)
#define MEMORY_BARRIER __sync_synchronize()
#elif defined(__GNUC__)
#define MEMORY_BARRIER __asm__ __volatile__ ("": : :"memory")
#else
#define MEMORY_BARRIER
#endif
#ifdef __GNUC__
/**
 * 条件式 x が true になる可能性が高いことをコンパイラに指示するマクロ．
 *
 * GCC 及び ICC でのみ有効です．
 *
 * @param[in] 条件式
 */
#define LIKELY(x)   __builtin_expect(!!(x), true)
/**
 * 条件式 x が false になる可能性が高いことをコンパイラに指示するマクロ．
 *
 * GCC 及び ICC でのみ有効です．
 *
 * @param[in] 条件式
 */
#define UNLIKELY(x) __builtin_expect(!!(x), false)
#else
#define LIKELY(x)   x
#define UNLIKELY(x) x
#endif
/**
 * apr_palloc のラッピングマクロ．
 *
 * メモリの確保に失敗すると例外を投げます．
 *
 * @param[out] value 確保したメモリ
 * @param[in] type 型
 * @param[in] pool プール
 * @param[in] size サイズ
 * @exception const char * メモリの確保に失敗した場合
 */
#define APR_PALLOC(value, type, pool, size)                             \
    if (UNLIKELY((value = reinterpret_cast<type>(apr_palloc(pool, size))) == NULL)) { \
        THROW(MESSAGE_SYS_MEMORY_ALLOC_FAILED);                           \
    }
/**
 * apr_pcalloc のラッピングマクロ．
 *
 * 確保した領域は 0 で初期化されています．
 * メモリの確保に失敗すると例外を投げます．
 *
 * @param[out] value 確保したメモリ
 * @param[in] type 型
 * @param[in] pool プール
 * @param[in] size サイズ
 * @exception const char * メモリの確保に失敗した場合
 */
#define APR_PCALLOC(value, type, pool, size)                            \
    if (UNLIKELY((value = reinterpret_cast<type>(apr_pcalloc(pool, size))) == NULL)) { \
        THROW(MESSAGE_SYS_MEMORY_ALLOC_FAILED);                         \
    }
/**
 * malloc のラッピングマクロ．
 *
 * メモリの確保に失敗するとプログラムを終了させします．
 *
 * @param[out] value 確保したメモリ
 * @param[in] type 型
 * @param[in] size サイズ
 * @param[in] number 個数
 */
#define MALLOC(value, type, size, number)                               \
    if (UNLIKELY((value = reinterpret_cast<type>(malloc(number * size))) == NULL)) { \
        fprintf(stderr, MESSAGE_SYS_MEMORY_ALLOC_FAILED);               \
        exit(EXIT_SUCCESS);                                             \
    }
/**
 * calloc のラッピングマクロ．
 *
 * メモリの確保に失敗するとプログラムを終了させします．
 *
 * @param[out] value 確保したメモリ
 * @param[in] type 型
 * @param[in] size サイズ
 * @param[in] number 個数
 */
#define CALLOC(value, type, size, number)                               \
    if (UNLIKELY((value = reinterpret_cast<type>(calloc(number, size))) == NULL)) { \
        fprintf(stderr, MESSAGE_SYS_MEMORY_ALLOC_FAILED);               \
        exit(EXIT_SUCCESS);                                             \
    }
/**
 * realloc のラッピングマクロ．
 *
 * メモリの確保に失敗するとプログラムを終了させします．
 *
 * @param[in,out] value 確保したメモリ
 * @param[in] type 型
 * @param[in] size サイズ
 * @param[in] number 個数
 */
#define REALLOC(value, type, size, number)                               \
    if (UNLIKELY((value = reinterpret_cast<type>(realloc(value, number * size))) == NULL)) { \
        fprintf(stderr, MESSAGE_SYS_MEMORY_ALLOC_FAILED);               \
        exit(EXIT_SUCCESS);                                             \
    }
/**
 * free のラッピングマクロ．
 *
 * メモリを解放します．
 *
 * @param[in] value 解放するメモリ
 */
#define FREE(value) free(value)
/**
 * 文字列定数の長さを返すマクロ．
 *
 * @param[in] str 文字列定数
 */
#define LITERAL_STRLEN(str) (sizeof(str) - 1)
/**
 * 配列の長さを返すマクロ．
 *
 * @param[in] array 配列
 */
#define ARRAY_SIZE_OF(array) (sizeof(array) / sizeof((array)[0]))
/**
 * ワードサイズを定義したマクロ．
 */
#define WORD_SIZE (sizeof(int))
/**
 * GCC の警告を避けるためのマクロ．
 *
 * 具体的には，ARM architecture 等で「increases required alignment of
 * target type」が出るのを防止します．
 *
 * @see gcc/c-typeck.c
 */
#ifdef __GNUC__
#define AVOID_ALIGN_WARN (void *)
#else
#define AVOID_ALIGN_WARN
#endif
/**
 * n * den が num 以上となる最小の n を計算するマクロ．
 *
 * @param[in] num 被除数
 * @param[in] den 除数
 */
#define PAD_DIV(num, den) ((num+den-1)/den)
/**
 * ポインタを char * にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_CHAR(pointer) reinterpret_cast<char *>(pointer)
/**
 * ポインタを unsigned char * にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_UCHAR(pointer) reinterpret_cast<unsigned char *>(pointer)
/**
 * ポインタを char ** にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_CHAR_P(pointer) reinterpret_cast<char **>(pointer)
/**
 * ポインタを const char * にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_CONST_CHAR(pointer) reinterpret_cast<const char *>(pointer)
/**
 * ポインタを const unsigned char * にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_CONST_UCHAR(pointer) reinterpret_cast<const unsigned char *>(pointer)
/**
 * ポインタを const char ** にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_CONST_CHAR_P(pointer) \
    const_cast<const char **>(reinterpret_cast<char **>(pointer))
/**
 * ポインタを byte * にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_BYTE(pointer) reinterpret_cast<apr_byte_t *>(pointer)
/**
 * ポインタを apr_uint32_t * にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_UINT32(pointer) reinterpret_cast<apr_uint32_t *>(pointer)
/**
 * ポインタを apr_uint16_t * にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_UINT16(pointer) reinterpret_cast<apr_uint16_t *>(pointer)
/**
 * ポインタを apr_size_t * にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_SIZE(pointer) reinterpret_cast<apr_size_t *>(pointer)
/**
 * ポインタを apr_off_t * にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_OFF(pointer) reinterpret_cast<apr_off_t *>(pointer)
/**
 * ポインタを apr_atomic_t * にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_ATOMIC(pointer) reinterpret_cast<apr_atomic_t *>(pointer)
/**
 * ポインタを apr_time_t * にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_TIME(pointer) reinterpret_cast<apr_time_t *>(pointer)
/**
 * ポインタを apr_status_t (*)(void *) にキャストするマクロ．
 *
 * @param[in] pointer ポインタ
 */
#define AS_CLEANUP_FUNC(pointer) reinterpret_cast<apr_status_t (*)(void *)>(pointer)
/**
 * エラー内容を表示して終了します．
 *
 * @param[in] str 内容
 */
#define FATAL_ERROR(str) do {                                          \
        cerr << str << " (" << __FILE__ << ":" << __LINE__ << ")";  \
        exit(EXIT_FAILURE);                                                     \
    } while (0)
#ifdef DEBUG
#define DUMP_COND(name, a, b) do {                                      \
        cerr << "\033[1;31m";                                           \
        cerr << "============================================================"; \
        cerr << endl;                                                   \
        cerr << "ASSERT ERROR: " << name << endl;                       \
        cerr << "------------------------------------------------------------"; \
        cerr << endl;                                                   \
        cerr << __FILE__ << ":" << __LINE__ << " @ ";                   \
        cerr << __FUNCTION__ << endl;                                   \
        cerr << "------------------------------------------------------------"; \
        cerr << endl;                                                   \
        cerr << setw(24) << setiosflags(ios::left) << APR_STRINGIFY(a); \
        cerr << " = " << (ptrdiff_t)(a) << endl;                        \
        cerr << setw(24) << setiosflags(ios::left) << APR_STRINGIFY(b); \
        cerr << " = " << (ptrdiff_t)(b) << endl;                        \
        cerr << "============================================================"; \
        cerr << "\033[0m" << endl;                                      \
        abort();                                                        \
    } while(0)
#endif
#ifdef DEBUG
/**
 * a と b が等しく無い場合に abort します．
 *
 * @param[in] a
 * @param[in] b
 */
#define ASSERT_EQUAL(a, b) do {                 \
        if ((a) != (b)) {                       \
            DUMP_COND("equal", a, b);           \
        }                                       \
    } while (0)
#else
#define ASSERT_EQUAL(a, b)
#endif
#ifdef DEBUG
/**
 * a が b よりも小さく無い場合に abort します．
 *
 * @param[in] a
 * @param[in] b
 */
#define ASSERT_LESS(a, b) do {                  \
        if ((a) >= (b)) {                       \
            DUMP_COND("less", a, b);            \
        }                                       \
    } while (0)
#else
#define ASSERT_LESS(a, b)
#endif

// strncpy_s のラッパ
#if _MSC_VER >= 1400
#define strncpy(dst, src, count) strncpy_s(dst, count, src, _TRUNCATE)
#endif

// sprintf_s のラッパ
#if _MSC_VER >= 1400
#undef snprintf
#define snprintf sprintf_s
#else
#ifdef _MSC_VER
#define snprintf _snprintf
#endif
#endif

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
