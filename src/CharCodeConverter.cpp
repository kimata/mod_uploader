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
 * $Id: CharCodeConverter.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#include "apu.h"

#if APU_HAVE_APR_ICONV
#include "apr_iconv.h"
#elif APU_HAVE_ICONV
#include <iconv.h>
#endif
#include "apr_strings.h"

#include "CharCodeConverter.h"
#include "Message.h"
#include "Macro.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: CharCodeConverter.cpp 2756 2007-12-11 10:57:59Z svn $");

const char CharCodeConverter::DEFAULT_CODE[]            = SYS_CHARACTER_CODE;
const unsigned char CharCodeConverter::EUCJP_PAT_FIRST  = 0xb5;
const unsigned char CharCodeConverter::SJIS_PAT_FIRST   = 0x8b;
const unsigned char CharCodeConverter::JIS_PAT_FIRST    = 0x1b;
const unsigned char CharCodeConverter::UTF8_PAT_FIRST   = 0xe4;

/******************************************************************************
 * public メソッド
 *****************************************************************************/
const char *CharCodeConverter::convert(apr_pool_t *pool, const char *str,
                                       const unsigned char *pat)
{
    return convert(pool, str, identify_code(pat), DEFAULT_CODE);
}

const char *CharCodeConverter::convert(apr_pool_t *pool, const char *str,
                                       const char *from, const char *to)
{
    char *converted;

#if APU_HAVE_APR_ICONV
    char *buffer;
    apr_size_t str_len = strlen(str);
    apr_size_t buffer_len = str_len * 3;
    apr_size_t converted_len = 0;
    apr_iconv_t handle;

    converted = buffer = static_cast<char *>(apr_pcalloc(pool, buffer_len));
    buffer_len--;

    if (apr_iconv_open(to, from, pool, &handle) != APR_SUCCESS) {
        throw apr_pstrcat(pool, MESSAGE_ICONV_CONVERTER_NOT_FOUND,
                          "(", from, " -> ", to, ")", NULL);
    }
    apr_iconv(handle,
              &str, &str_len, &buffer, &buffer_len, &converted_len);
    apr_iconv_close(handle, pool);
#elif APU_HAVE_ICONV
    char *buffer;
    ICONV_2ND_ARG_TYPE target;
    size_t str_len = strlen(str);
    size_t buffer_len = str_len * 3;
    iconv_t handle;

    target = static_cast<ICONV_2ND_ARG_TYPE>(apr_pstrdup(pool, str));
    converted = buffer = static_cast<char *>(apr_pcalloc(pool, buffer_len));
    buffer_len--;

    if ((handle = iconv_open(to, from)) == reinterpret_cast<iconv_t>(-1)) {
        throw apr_pstrcat(pool, MESSAGE_ICONV_CONVERTER_NOT_FOUND,
                          "(", from, " -> ", to, ")", NULL);
    }
    iconv(handle, &target, &str_len, &buffer, &buffer_len);
    iconv_close(handle);
#else

    // iconv，apr_iconv が無い場合，文字コード変換は行わない
    converted = apr_pstrdup(pool, str);
#endif

    return converted;
}

#include <stdio.h>

/******************************************************************************
 * private メソッド
 *****************************************************************************/
const char *CharCodeConverter::identify_code(const unsigned char *pat)
{
    switch (*pat) {
    case EUCJP_PAT_FIRST:   return "euc-jp";
    case SJIS_PAT_FIRST:    return "cp932";
    case JIS_PAT_FIRST:     return "iso-2022-jp";
    case UTF8_PAT_FIRST:    return "utf-8";
    case '\0':              return DEFAULT_CODE;
    default:
        THROW(MESSAGE_ICONV_ENCODING_INVALID);
    }
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_CharCodeConverter
#include "TestRunner.h"

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << " <FROM_CODE> <TO_CODE> <STRING>" << endl;
}

void run_convert(apr_pool_t *pool,
                 const char *from_code, const char *to_code, const char *str)
{
    const char *converted;

    show_test_name("convert");

    converted = CharCodeConverter::convert(pool, str, from_code, to_code);

    show_item("converted", converted);
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    const char *from_code;
    const char *to_code;
    const char *str;

    if (argc != 4) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    from_code = argv[1];
    to_code   = argv[2];
    str       = argv[3];

    show_item("from", from_code);
    show_item("to", to_code);
    show_item("str", str);
    show_line();

    run_convert(pool, from_code, to_code, str);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
