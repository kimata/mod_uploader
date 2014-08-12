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
 * $Id: uploader_util.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef UPLOADER_UTIL_H
#define UPLOADER_UTIL_H

#include "Environment.h"

#include "apr_tables.h"

#include "RFC1867Parser.h"
#include "Auxiliary.h"
#include "Message.h"

#define is_text_content(content)                            \
    ((content != NULL) &&                                   \
     (content->type == MultipartMessageParser<R>::TEXT))
#define is_file_content(content)                            \
    ((content != NULL) &&                                   \
     (content->type == MultipartMessageParser<R>::FILE))

class UploaderConfig;

static const char POST_ID_PARAM[]                   = "id";
static const char POST_FILE_PARAM[]                 = "file";
static const char POST_COMMENT_PARAM[]              = "comment";
static const char POST_REMOVE_PASS_PARAM[]          = "remove_pass";
static const char POST_DOWNLOAD_PASS_PARAM[]        = "download_pass";
static const char POST_CODE_PAT_PARAM[]             = "code_pat";

static const char MAIL_LINE_SEPARATOR[]             = "\r\n";

static const char ARG_SEPARATE_STR[]                = "/";

static const char FILE_EXT_SEPARATOR[]              = ".";

const char *get_word(apr_pool_t *pool, const char **input,
                     const char delimiter);

void get_page_count(apr_size_t item_count, apr_size_t per_page_item_number,
                    apr_size_t *page_count);
void get_page(apr_pool_t *pool, const char *arg, apr_size_t page_count,
              apr_size_t *page_no);

template<class R>
void get_upload_param(apr_pool_t *pool, apr_array_header_t *content_array,
                      const char **file_name, const char **file_mime,
                      apr_uint64_t *file_size, const char **file_digest,
                      const char **file_temp_path, const char **remove_pass,
                      const char **download_pass, const char **comment,
                      const char **code_pat)
{
    typename RFC1867Parser<R>::content_t *file_content;
    typename RFC1867Parser<R>::content_t *comment_content;
    typename RFC1867Parser<R>::content_t *remove_pass_content;
    typename RFC1867Parser<R>::content_t *download_pass_content;
    typename RFC1867Parser<R>::content_t *code_pat_content;

    file_content = MultipartMessageParser<R>::get_content
        (content_array, POST_FILE_PARAM);
    comment_content = MultipartMessageParser<R>::get_content
        (content_array, POST_COMMENT_PARAM);
    remove_pass_content = MultipartMessageParser<R>::get_content
        (content_array, POST_REMOVE_PASS_PARAM);
    download_pass_content = MultipartMessageParser<R>::get_content
        (content_array, POST_DOWNLOAD_PASS_PARAM);
    code_pat_content = MultipartMessageParser<R>::get_content
        (content_array, POST_CODE_PAT_PARAM);

    if (!is_file_content(file_content)) {
        THROW(MESSAGE_POST_DATA_INVALID);
    }

    *file_name      = file_content->file.name;
    *file_mime      = file_content->file.mime;
    *file_size      = file_content->file.size;
    *file_digest    = file_content->file.digest;
    *file_temp_path = file_content->file.temp_path;

    *comment = is_text_content(comment_content) ?
        *comment = comment_content->text : "";
    *remove_pass = is_text_content(remove_pass_content) ?
        *remove_pass = remove_pass_content->text : "";
    *download_pass = is_text_content(download_pass_content) ?
        *download_pass = download_pass_content->text : "";
    *code_pat = is_text_content(code_pat_content) ?
        *code_pat = code_pat_content->text : "";
}

template<class R>
void get_mail_upload_param(apr_pool_t *pool, apr_array_header_t *content_array,
                           const char **file_name, const char **file_mime,
                           apr_uint64_t *file_size, const char **file_digest,
                           const char **file_temp_path, const char **remove_pass,
                           const char **download_pass, const char **comment)
{
    typename RFC1867Parser<R>::content_t *text_content;
    typename RFC1867Parser<R>::content_t *file_content;
    const char *line_start;
    const char *line_end;

    text_content = MultipartMessageParser<R>::get_text_content(content_array);
    file_content = MultipartMessageParser<R>::get_file_content(content_array);

    if ((text_content == NULL) || (file_content == NULL)) {
        THROW(MESSAGE_POST_DATA_INVALID);
    }

    *file_name      = file_content->file.name;
    *file_mime      = file_content->file.mime;
    *file_size      = file_content->file.size;
    *file_digest    = file_content->file.digest;
    *file_temp_path = file_content->file.temp_path;

    *remove_pass    = "";
    *download_pass  = "";
    *comment        = "";

    line_start = text_content->text;
    line_end = strnchr(line_start, strlen(line_start), MAIL_LINE_SEPARATOR[0]);
    if (line_end == NULL) {
        *comment = line_start;
        return;
    }
    *comment = apr_pstrmemdup(pool, line_start, line_end-line_start);

    line_start = line_end + 2;
    line_end = strnchr(line_start, strlen(line_start), MAIL_LINE_SEPARATOR[0]);
    if (line_end == NULL) {
        *remove_pass = line_start;
        return;
    }
    *remove_pass = apr_pstrmemdup(pool, line_start, line_end-line_start);

    line_start = line_end + 2;
    line_end = strnchr(line_start, strlen(line_start), MAIL_LINE_SEPARATOR[0]);
    if (line_end == NULL) {
        *download_pass = line_start;
        return;
    }
    *download_pass = apr_pstrmemdup(pool, line_start, line_end-line_start);
}

template<class R>
const char *get_download_password(apr_pool_t *pool,
                                  apr_array_header_t *content_array)
{
    typename RFC1867Parser<R>::content_t *content;

    content = MultipartMessageParser<R>::get_content(content_array,
                                                     POST_DOWNLOAD_PASS_PARAM);

    if (!is_text_content(content)) {
        THROW(MESSAGE_POST_DATA_INVALID);
    }

    return content->text;
}

template<class R>
void get_remove_param(apr_pool_t *pool, apr_array_header_t *content_array,
                      apr_size_t *item_id, const char **remove_pass)
{
    typename RFC1867Parser<R>::content_t *id_content;
    typename RFC1867Parser<R>::content_t *remove_pass_content;

    id_content = MultipartMessageParser<R>::get_content
        (content_array, POST_ID_PARAM);
    remove_pass_content = MultipartMessageParser<R>::get_content
        (content_array, POST_REMOVE_PASS_PARAM);

    if (!is_text_content(id_content)) {
        THROW(MESSAGE_POST_DATA_INVALID);
    }

    *item_id = static_cast<apr_size_t>(apr_atoi64(id_content->text));
    *remove_pass = remove_pass_content->text;
}

bool can_post(UploaderConfig *config, apr_sockaddr_t *ip_address);
void regist_post(UploaderConfig *config, apr_sockaddr_t *ip_address);

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
