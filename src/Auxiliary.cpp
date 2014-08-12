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
 * $Id: Auxiliary.cpp 2880 2008-05-01 12:19:40Z svn $
 *****************************************************************************/

#include "Environment.h"

#include "Auxiliary.h"
#include "Message.h"
#include "Macro.h"

#include "AtomicWrapper.h"
#include "apr_file_info.h"
#include "apr_strings.h"
#include "apr_time.h"
#include "apr_errno.h"

#include <cctype>
#include <cstring>
#include <iostream>

#include <time.h>

#include <cstdio>

#include "SourceInfo.h"

using namespace std;

SOURCE_INFO_ADD("$Id: Auxiliary.cpp 2880 2008-05-01 12:19:40Z svn $");

const char *rfc2396_encode(apr_pool_t *pool, const char* str)
{
    static char hex[] = "0123456789ABCDEF";

    apr_size_t conv_num;
    char *encoded;
    char *buffer;

    conv_num = 0;
    for (const char *plain = str; *plain != '\0'; plain++) {
        if (isalnum(*plain) ||
            (*plain == '.') || (*plain == '-') || (*plain == '_')) {
            continue;
        }
        conv_num++;
    }

    if (conv_num == 0) {
        return apr_pstrdup(pool, str);
    }

    APR_PCALLOC(encoded, char *, pool, strlen(str) + (conv_num*2) + 1);
    buffer = encoded;

    for (; *str != '\0'; str++) {
        if (isalnum(*str) || (*str == '.') || (*str == '-') || (*str == '_')) {
            *buffer++ = *str;
        } else {
            const unsigned char c = *str;

            *buffer++ = '%';
            *buffer++ = hex[c / 0x10];
            *buffer++ = hex[c % 0x10];
        }
    }
    *buffer = '\0';

    return encoded;
}

void init_atomic(apr_pool_t *pool)
{
    if (apr_atomic_init(pool) != APR_SUCCESS) {
        THROW(MESSAGE_SYS_ATOMIC_INITIALIZE_FAILED);
    }
}

apr_shm_t *create_shm(apr_pool_t *pool, apr_size_t size, const char *file_path)
{
    apr_shm_t *shm;

    if (apr_shm_create(&shm, size, file_path, pool) != APR_SUCCESS) {
        THROW(MESSAGE_SHM_CREATION_FAILED);
    }

    return shm;
}

void attach_shm(apr_pool_t *pool, apr_shm_t *shm, const char *file_path)
{
    // Anonymous Shared Memory の場合は不要
    if (file_path == NULL) {
        return;
    }

    if (apr_shm_attach(&shm, file_path, pool) != APR_SUCCESS) {
        THROW(MESSAGE_SHM_ATTACH_FAILED);
    }
}

void check_apr_error(apr_pool_t *pool, apr_status_t status)
{
    if (status == APR_SUCCESS) {
        return;
    }

    cerr << "APR Error: ";
    cerr << get_error_message(pool, status);
    cerr << endl;
}

const char *get_error_message(apr_pool_t *pool, apr_status_t status)
{
    static const apr_size_t BUFFER_SIZE = 512;
    char *buffer;

    APR_PALLOC(buffer, char *, pool, BUFFER_SIZE);

    return apr_strerror(status, buffer, BUFFER_SIZE);
}

const char *get_last_error_message(apr_pool_t *pool)
{
    return get_error_message(pool, APR_FROM_OS_ERROR(apr_get_os_error()));
}

double get_time_sec()
{
    return static_cast<double>(apr_time_now())/APR_USEC_PER_SEC;
}

#ifdef WIN32
#include <Windows.h>
double get_usage_sec()
{
    LARGE_INTEGER time, freq;

    QueryPerformanceCounter(&time);
    QueryPerformanceFrequency(&freq);

    return static_cast<double>(time.QuadPart) / freq.QuadPart;
}
#else
#include <sys/time.h>
#include <sys/resource.h>
double get_usage_sec()
{
    struct rusage rusage;

    getrusage(RUSAGE_SELF, &rusage);

    return (double)rusage.ru_utime.tv_sec + (double)rusage.ru_utime.tv_usec*(1e-6);
}
#endif

apr_uint32_t mask32(apr_uint32_t x)
{
    x = x - 1;
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);

    return x;
}

apr_uint32_t clip32(apr_uint32_t x)
{
    return mask32(x) + 1;
}

apr_size_t bit_width(apr_size_t x)
{
    apr_size_t j;

    j = x;
    for (apr_size_t i = 0; i < (sizeof(apr_size_t)*8); i++) {
        if (j == 0) {
            return i;
        }

        j >>= 1;
    }

    return (sizeof(apr_size_t) * 8);
}

apr_size_t atosize(const char *s)
{
    return static_cast<apr_size_t>(apr_atoi64(s));
}

const char *get_file_ext(const char *file_name)
{
    const char *file_ext;
    apr_size_t name_length;

    name_length = strlen(file_name);
    if (name_length == 0 ) {
        return "";
    }

    for (file_ext = file_name+name_length-1; file_ext != file_name;
         file_ext--) {
        if (*file_ext == '.') {
            return ++file_ext;
        }
        if (!isalnum(static_cast<unsigned char>(*file_ext))) {
            return "";
        }
    }

    return "";
}

const char *get_mime_image_ext(const char *file_mime)
{
    if (strcmp(file_mime, "image/bmp") == 0) {
        return "bmp";
    } else if (strcmp(file_mime, "image/gif") == 0) {
        return "gif";
    } else if (strcmp(file_mime, "image/jpeg") == 0) {
        return "jpg";
    } else if (strcmp(file_mime, "image/pjpeg") == 0) {
        return "jpg";
    } else if (strcmp(file_mime, "image/png") == 0) {
        return "png";
    } else {
        THROW(MESSAGE_FILE_MIME_INVALID);
    }
}

const char *get_image_mime(const char *file_name)
{
    const char *file_ext;

    file_ext = get_file_ext(file_name);

    if (strcmp(file_ext, "bmp") == 0) {
        return "image/bmp";
    } else if (strcmp(file_ext, "gif") == 0) {
        return "image/gif";
    } else if (strcmp(file_ext, "jpg") == 0) {
        return "image/jpg";
    } else if (strcmp(file_ext, "png") == 0) {
        return "image/png";
    } else {
        THROW(MESSAGE_FILE_EXT_INVALID);
    }
}

const char *basename_ex(const char *path)
{
    const char *i = path + strlen(path) - 2;

    while ((*i != '/') && (*i != '\\') && (*i != ':') &&
           (*i != '\r') && (*i != '\n')) { /* 念のため */
        if (i == path) {
            return path;
        }

        i--;
    }

    return ++i;
}

const char *dirname_ex(apr_pool_t *pool, const char *path)
{
    char *dirname;
    char *i;

    dirname = apr_pstrdup(pool, path);

    i = dirname + strlen(dirname) - 2;
    while ((*i != '/') && (*i != '\\') && (*i != ':') &&
           (*i != '\r') && (*i != '\n')) {
        if (i == path) {
            break;
        }

        i--;
    }

    *i = '\0';

    return dirname;
}

void escape_html(apr_pool_t *pool,
                 const char *str, apr_size_t str_length,
                 const char **escaped_str, apr_size_t *escaped_length)
{
    static const char LESS_THAN[]       = "&lt;";
    static const char GREATER_THAN[]    = "&gt;";
    static const char AMPERSAND[]       = "&amp;";

    apr_size_t result_length;
    char *buffer;

#ifdef DEBUG
    if (str_length != strlen(str)) {
        THROW(MESSAGE_BUG_FOUND);
    }
#endif

    result_length = str_length;

    for (const char *plain = str; *plain != '\0'; plain++) {
        if (*plain == '<') {
            result_length += LITERAL_STRLEN(LESS_THAN) - 1;
        } else if (*plain == '>') {
            result_length += LITERAL_STRLEN(GREATER_THAN) - 1;
        } else if (*plain == '&') {
            result_length += LITERAL_STRLEN(AMPERSAND) - 1;
        }
    }

    *escaped_length = result_length;
    if (result_length == 0) {
        *escaped_str = apr_pstrndup(pool, str, str_length);
        return;
    }
    APR_PALLOC(buffer, char *, pool, result_length + 1);
    *escaped_str = buffer;

    for (; *str != '\0'; str++) {
        if (*str == '<') {
            memcpy(buffer, LESS_THAN, LITERAL_STRLEN(LESS_THAN));
            buffer += LITERAL_STRLEN(LESS_THAN);
        } else if (*str == '>') {
            memcpy(buffer, GREATER_THAN, LITERAL_STRLEN(GREATER_THAN));
            buffer += LITERAL_STRLEN(GREATER_THAN);
        } else if (*str == '&') {
            memcpy(buffer, AMPERSAND, LITERAL_STRLEN(AMPERSAND));
            buffer += LITERAL_STRLEN(AMPERSAND);
        } else {
            *buffer++ = *str;
        }
    }

    *buffer = '\0';
}

const char *escape_html(apr_pool_t *pool, const char *str)
{
    const char *escaped_str;
    apr_size_t escaped_length;

    escape_html(pool, str, strlen(str), &escaped_str, &escaped_length);

    return escaped_str;
}

const char *size_str(apr_pool_t *pool, apr_uint64_t size)
{
    static const apr_size_t CMP_FACTOR  = 1024;
    apr_size_t temp_size;
    const char *unit_str;

    // MEMO: 約 400 TB 以上だと桁あふれして誤動するので注意

    do {
        if (size > (1024LL*1024*1024)) {
            temp_size = static_cast<apr_size_t>(size /
                                                (1024LL*1024*1024/CMP_FACTOR));
            unit_str = GBYTE_UNIT;
            break;
        }

        temp_size = static_cast<apr_size_t>(size / (1024/CMP_FACTOR));

        if (temp_size > (1024 * CMP_FACTOR)) {
            temp_size /= 1024;
            unit_str = MBYTE_UNIT;
        } else if (temp_size > static_cast<apr_size_t>(CMP_FACTOR)) {
            unit_str = KBYTE_UNIT;
        } else {
            temp_size = static_cast<apr_size_t>(size * CMP_FACTOR);
            unit_str = BYTE_UNIT;
        }
    } while (0);

    return apr_psprintf(pool, "%" APR_SIZE_T_FMT ".%" APR_SIZE_T_FMT " %s",
                        temp_size / CMP_FACTOR,
                        (temp_size * 10 / CMP_FACTOR) % 10,
                        unit_str);
}

#ifndef HAVE_STRNCHR
const char *strnchr(const char *s, size_t length, int c)
{
    const char *pos;
    const unsigned long *word_pos;
    unsigned long word_char;
    unsigned long magic;
    unsigned long cmask;
    apr_size_t count;
    apr_size_t tail_count;

    pos = const_cast<char *>(s);
    count = static_cast<apr_size_t>(length);

    // ワード境界まではバイト単位でチェック．
    tail_count = static_cast<apr_size_t>(reinterpret_cast<uintptr_t>(pos) &
                                         (sizeof(long)-1));
    if (tail_count) {
        tail_count = sizeof(long) - tail_count;
        while (tail_count--) {
            if (count-- == 0) {
                return NULL;
            }
            if (*pos == c) {
                return pos;
            } else if (*pos++ == '\0') {
                return NULL;
            }
        }
    }

    word_pos = reinterpret_cast<const unsigned long *>(AVOID_ALIGN_WARN pos);

    if (sizeof(long) == 4) {
        magic = 0x7EFEFEFFL;
        tail_count = count & 0x3;
    } else if (sizeof(long) == 8) {
        magic = ((0x7EFEFEFEL << 16) << 16) | 0xFEFEFEFFL;
        tail_count = count & 0x7;
    } else {
        abort();
    }
    count -= tail_count;

    cmask = c | (c << 8);
    cmask |= cmask << 16;
    if (sizeof(long) == 8) {
        cmask |= (cmask << 16) << 16;
    }

    // ワード単位でチェック．
    for (; count > 0; count -= sizeof(long)) {
        word_char = *word_pos++;

        if (LIKELY((((((word_char) +
                       magic) ^ ~(word_char)) & ~magic) == 0) &&
                   (((((word_char ^ cmask) +
                       magic) ^ ~(word_char ^ cmask)) & ~magic) == 0))) {
            continue;
        }

        pos = reinterpret_cast<const char *>(word_pos - 1);

        for (apr_size_t i = sizeof(long); i != 0; i--) {
            if (*pos == c) {
                return pos;
            } else if (*pos++ == '\0') {
                return NULL;
            }
        }
    };

    // Valgrind で
    // 「Conditional jump or move depends on uninitialised value(s)」
    // が発生するのが嫌な感じなので，末尾の半端な部分については 1 文字
    // ずつチェックする．
    pos = reinterpret_cast<const char *>(word_pos);
    while (tail_count--) {
        if (*pos == c) {
            return pos;
        } else if (*pos++ == '\0') {
            return NULL;
        }
    }

    return NULL;
}
#endif

#if !defined(HAVE_MEMMEM) || defined(WIN32)
void *memmem(const void *haystack, size_t haystacklen,
             const void *needle, size_t needlelen)
{
    unsigned char *start = (unsigned char *)haystack;
    unsigned char pat = *((unsigned char *)needle);
    unsigned char *pos  = start;
    apr_size_t remain = haystacklen;

    do {
        pos = (unsigned char *)memchr(pos, pat, remain);

        if (pos == NULL) {
            return NULL;
        }

        if ((memcmp(pos, needle, needlelen)) == 0) {
            return pos;
        }
        pos++;

        remain = haystacklen - (pos-start);
    } while (remain >= needlelen);

    return NULL;
}
#endif

#if !defined(HAVE_STRNDUP) || defined(WIN32)
char *strndup(const char *s, size_t n)
{
    char *dup_str;

    MALLOC(dup_str, char *, sizeof(char), n + 1);
    memcpy(dup_str, s, n);
    *(dup_str + n) = '\0';

    return dup_str;
}
#endif

int diff(apr_pool_t *pool, const char *a_file_path, const char *b_file_path)
{
    // お手軽に
    return system(apr_pstrcat(pool, "diff -q ", a_file_path, " ", b_file_path,
                              NULL));
    return 0;
}

/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_Auxiliary
#include "TestRunner.h"

const char *size_str_ref(apr_pool_t *pool, apr_uint64_t size)
{
    double size_num;
    const char *unit_str;

    if (size > (1024*1024*1024)) {
        size_num = (double)(size / (1024*1024)) / 1024;
        unit_str = GBYTE_UNIT;
    } else if (size > (1024*1024)) {
        size_num = (double)(size / (1024)) / 1024;
        unit_str = MBYTE_UNIT;
    } else if (size > 1024) {
        size_num = (double)size / 1024;
        unit_str = KBYTE_UNIT;
    } else {
        size_num = (double)size;
        unit_str = BYTE_UNIT;
    }

    // 小数点以下第一位で切り捨て
    return apr_psprintf(pool, "%.1f %s",
                        (apr_size_t)(size_num * 10) / 10.0, unit_str);
}

void show_usage(const char *prog_name)
{

    cerr << "Usage: " << prog_name << endl;
}

void run_size_str(apr_pool_t *pool)
{
    static const apr_size_t TEST_COUNT  = 100000;
    apr_uint64_t num;
    const char *str;
    const char *str_ref;
    volatile double start_time;
    volatile double end_time;

    show_test_name("size_str");

    start_time = get_usage_sec();
    for (apr_size_t i = 0; i < TEST_COUNT; i++) {
        num = rand();
        str = size_str(pool, num);
        str_ref = size_str_ref(pool, num);

        if (strcmp(str, str_ref)) {
            THROW(MESSAGE_BUG_FOUND);
        }
    }
    end_time = get_usage_sec();

    show_item("size_str", (end_time - start_time) * 1000*1000/TEST_COUNT,
              " usec/call");
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    if (argc != 1) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    show_line();
    run_size_str(pool);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
