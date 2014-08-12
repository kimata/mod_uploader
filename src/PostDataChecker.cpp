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
 * $Id: PostDataChecker.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <cstring>

#include "PostDataChecker.h"
#include "CharCodeConverter.h"
#include "Auxiliary.h"
#include "Message.h"
#include "Macro.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: PostDataChecker.cpp 2756 2007-12-11 10:57:59Z svn $");

const char PostDataChecker::HTML_EXT[]      = "htm";

/******************************************************************************
 * public メソッド
 *****************************************************************************/
void PostDataChecker::validate_uitem(apr_pool_t *pool, apr_uint64_t file_size,
                                     apr_time_t  mtime, const char **file_name,
                                     const char **file_mime,
                                     const char *file_digest,
                                     const char *remove_pass,
                                     const char *download_pass,
                                     const char **comment,
                                     const char *code_pat)
{
    const char *file_ext;

    validate_file_name(pool, file_name, code_pat);
    validate_file_mime(pool, *file_mime);
    validate_comment(pool, comment, code_pat);
    file_ext = get_file_ext(*file_name);
    validate_file_ext(file_ext);

    if (strlen(*file_name) == 0) {
        THROW(MESSAGE_POST_FILE_NAME_EMPTY);
    } else if (strlen(*file_name) > (ITM_MAX_FILE_NAME_SIZE-1)) {
        THROW(MESSAGE_POST_FILE_NAME_TOO_LONG);
    }
    if (file_size == 0) {
        THROW(MESSAGE_POST_FILE_SIZE_ZERO);
    }
    if (strlen(*file_mime) == 0) {
        THROW(MESSAGE_POST_FILE_MIME_EMPTY);
    } else if (strlen(*file_mime) > (ITM_MAX_FILE_MIME_SIZE-1)) {
        THROW(MESSAGE_POST_FILE_MIME_TOO_LONG);
    }
    if (strlen(file_ext) == 0) {
        THROW(MESSAGE_POST_FILE_EXT_EMPTY);
    } else if (strlen(file_ext) > (ITM_MAX_FILE_EXT_SIZE-1)) {
        THROW(MESSAGE_POST_FILE_EXT_TOO_LONG);
    }
    if (strlen(file_digest) == 0) {
        THROW(MESSAGE_POST_FILE_DIGEST_EMPTY);
    } else if (strlen(file_digest) > (ITM_MAX_FILE_DIGEST_SIZE-1)) {
        THROW(MESSAGE_POST_FILE_DIGEST_TOO_LONG);
    }
    if (strlen(remove_pass) > (ITM_MAX_REMOVE_PASS_SIZE-1)) {
        THROW(MESSAGE_POST_REMOVE_PASS_TOO_LONG);
    }
    if (strlen(download_pass) > (ITM_MAX_DOWNLOAD_PASS_SIZE-1)) {
        THROW(MESSAGE_POST_DOWNLOAD_PASS_TOO_LONG);
    }

    if (strlen(*comment) == 0) {
#ifndef EMPTY_COMMENT
        THROW(MESSAGE_POST_COMMENT_EMPTY);
#endif
    } else if (strlen(*comment) > (ITM_MAX_COMMENT_SIZE-1)) {
        THROW(MESSAGE_POST_COMMENT_TOO_LONG);
    }
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void PostDataChecker::validate_file_name(apr_pool_t *pool,
                                         const char **file_name,
                                         const char *code_pat)
{
    if (strlen(*file_name) == 0) {
        return;
    }

    // 不正なシーケンスを排除
    *file_name = CharCodeConverter::convert
        (pool, *file_name,
         reinterpret_cast<const unsigned char *>(code_pat));

    // 良い方法じゃないけど...
    for (const char *name = *file_name; *name != '\0'; name++) {
        if ((*name == '<') || (*name == '>') || (*name == '"') ||
            (*name == '\'') || (*name == '\r') || (*name == '\n') ||
            (*name == '\\')) {
            THROW(MESSAGE_POST_FILE_NAME_INVALID);
        }
    }
}

void PostDataChecker::validate_file_mime(apr_pool_t *pool,
                                         const char *file_mime)
{
    for (const char *mime = file_mime; *mime != '\0'; mime++) {
        if (isalnum(*mime) || (*mime == '-') || (*mime == '+') ||
            (*mime == '.') || (*mime == '/') || (*mime == ';') ||
            (*mime == '=') || (*mime == ' ')) {
            continue;
        }

        THROW(MESSAGE_POST_FILE_MIME_INVALID);
    }
}

void PostDataChecker::validate_comment(apr_pool_t *pool, const char **comment,
                                       const char *code_pat)
{
    apr_size_t escaped_length;

    if (strlen(*comment) == 0) {
        return;
    }

    // 不正なシーケンスを排除
    *comment = CharCodeConverter::convert
        (pool, *comment,
         reinterpret_cast<const unsigned char *>(code_pat));

    escape_html(pool, *comment, strlen(*comment), comment, &escaped_length);
}

void PostDataChecker::validate_file_ext(const char *file_ext)
{
    if (strncmp(file_ext, HTML_EXT, LITERAL_STRLEN(HTML_EXT)) == 0) {
        THROW(MESSAGE_POST_FILE_EXT_INVALID);
    }
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
