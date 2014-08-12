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
 * $Id: RFC1867Parser.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef TEMPLATE_INSTANTIATION
#include "Environment.h"
#endif

#include "RFC1867Parser.h"
#include "MultipartMessageParser.h"
#include "TemporaryFile.h"
#include "DirectoryCleaner.h"
#include "Auxiliary.h"
#include "Message.h"
#include "Macro.h"
#include "SourceInfo.h"

using namespace std;

#ifndef TEMPLATE_INSTANTIATION
SOURCE_INFO_ADD("$Id: RFC1867Parser.cpp 2756 2007-12-11 10:57:59Z svn $");
#endif

#ifdef DEBUG_RFC1867Parser
#define DUMP_INPUT_AND_THROW(message) M::dump_input(M::buffer_); THROW(message)
#else
#define DUMP_INPUT_AND_THROW(message) THROW(message)
#endif

#ifdef DEBUG_RFC1867Parser
static bool parser_is_trace = false;
#define DUMP_TOKEN_AND_THROW(message) dump_read_token(); THROW(message)
#ifdef _MSC_VER
#define TRACE_FUNC if (parser_is_trace) cerr << "CALL: " << __LINE__ << endl
#else
#define TRACE_FUNC if (parser_is_trace) cerr << "CALL: " << __func__ << endl
#endif
#else
#define DUMP_TOKEN_AND_THROW(message) THROW(message)
#define TRACE_FUNC
#endif

template<class R, class W> const char
RFC1867Parser<R, W>::MULTIPART_FORM_DATA[]  = "multipart/form-data; ";
template<class R, class W> const char
RFC1867Parser<R, W>::FORM_DATA[]            = "form-data; ";
template<class R, class W> const char
RFC1867Parser<R, W>::FILENAME_PARAM[]       = "filename";

/******************************************************************************
 * public メソッド
 *****************************************************************************/
template<class R, class W>
RFC1867Parser<R, W>::RFC1867Parser(apr_pool_t *pool, PostReaderClass& reader,
                                   const char *file_dir_path,
                                   apr_size_t max_text_size,
                                   apr_uint64_t max_file_size,
                                   apr_size_t max_item_num,
                                   apr_size_t file_offset)
  : M(pool, reader, file_dir_path, max_text_size, max_file_size, max_item_num,
      file_offset)
{

}

template<class R, class W>
apr_array_header_t *RFC1867Parser<R, W>::parse(const char *content_type,
                                               apr_uint64_t content_size)
{
    apr_array_header_t *content_array;
    content_t content;
    const char *start;

    TRACE_FUNC;

    // 古い一時ファイルを削除
    DirectoryCleaner::clean_old_files(M::pool_, M::file_dir_path_,
                                      M::READ_TIMEOUT_SEC);

    // "仮の" サイズチェック
    if (content_size > ((M::max_text_size_+M::max_file_size_) *
                        M::max_item_num_)) {
        THROW(MESSAGE_RFC1867_DATA_SIZE_TOO_LARGE);
    }

    content_array = apr_array_make(M::pool_,
                                   static_cast<int>(M::max_item_num_),
                                   static_cast<int>(sizeof(content_t)));

    M::boundary_ = get_boundary(content_type);
    M::boundary_len_ = strlen(M::boundary_);
    M::barrier_len_ = M::boundary_len_ + strlen(M::CR_LF) +
        strlen(M::BOUNDARY_PREFIX);

    if (M::fill() == 0) {
        THROW(MESSAGE_RFC1867_CONTENT_SIZE_ZERO);
    }

    start = M::skip_line(M::buffer_.get_data()) - strlen(M::CR_LF);
    M::buffer_.erase(start - M::buffer_.get_data());

    while (!M::is_end()) {
        if (static_cast<apr_size_t>(content_array->nelts) == M::max_item_num_) {
            THROW(MESSAGE_RFC1867_ITEM_COUNT_EXCEEDED);
        }

        get_content(&content);
        *AS_CONTENT(apr_array_push(content_array)) = content;
    }

    return content_array;
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
template<class R, class W>
void RFC1867Parser<R, W>::get_content(content_t *content)
{
    const char *start;
    const char *line_end;

    TRACE_FUNC;

    if ((M::fill() == 0) && (M::buffer_.get_size() == 0)) {
        DUMP_INPUT_AND_THROW(MESSAGE_RFC1867_FORMAT_INVALID);
    }

    if (!start_with(M::buffer_.get_data(), M::CR_LF)) {
        DUMP_INPUT_AND_THROW(MESSAGE_RFC1867_FORMAT_INVALID);
    }

    start = M::buffer_.get_data();
    start += strlen(M::CR_LF);

    line_end = M::skip_line(start);

    start = M::skip(start, M::CONTENT_DISPOSITION);
    start = M::skip(start, FORM_DATA);

    start = get_param(start, line_end, M::NAME_PARAM, &(content->name));

    if (start == NULL) {
        DUMP_INPUT_AND_THROW(MESSAGE_RFC1867_FORMAT_INVALID);
    }

    start = get_param(start, line_end, FILENAME_PARAM,
                      &(content->file.name));

    if (start == NULL) { // テキスト
        start = M::skip_header(line_end - LITERAL_STRLEN(M::CR_LF));
        M::buffer_.erase(start - M::buffer_.get_data());

        M::get_text_content(content);
    } else { // ファイル
        content->file.name = basename_ex(content->file.name);

        start = M::skip_line(start);
        line_end = M::skip_line(start);

        start = M::skip(start, M::CONTENT_TYPE);
        content->file.mime
            = AS_CONST_CHAR(apr_pstrmemdup(M::pool_, start,
                                           line_end - start -
                                           LITERAL_STRLEN(M::CR_LF)));

        start = M::skip_header(line_end - LITERAL_STRLEN(M::CR_LF));
        M::buffer_.erase(start - M::buffer_.get_data());

        M::get_file_content(content);
    }
}

template<class R, class W>
const char *RFC1867Parser<R, W>::get_boundary(const char *content_type)
{
    const char *boundary;
    const char *start;
    const char *end;

    TRACE_FUNC;

    start = M::skip(content_type, MULTIPART_FORM_DATA);
    end = content_type+strlen(content_type);

    if (get_param(start, end, M::BOUNDARY_PARAM, &boundary) == NULL) {
        THROW(MESSAGE_RFC1867_CONTENT_TYPE_INVALID);
    }

    if (strlen(boundary) <= strlen(M::BOUNDARY_PREFIX)) {
        THROW(MESSAGE_RFC1867_CONTENT_TYPE_INVALID);
    } else if (strlen(boundary) > (M::READ_BLOCK_SIZE/2)) {
        THROW(MESSAGE_RFC1867_CONTENT_TYPE_INVALID);
    }

    return boundary;
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_RFC1867Parser
#include <fstream>

#include "File.h"
#include "CGIRequestReader.h"

#define TEMPLATE_INSTANTIATION
#include "MultipartMessageParser.cpp"

#include "TestRunner.h"

static const char FILE_CONTENT_NAME[]   = "file";
static const apr_size_t MAX_TEXT_SIZE   = 100 * 1024;
static const apr_size_t MAX_FILE_SIZE   = 100 * 1024 * 1024;
static const apr_size_t MAX_ITEM_NUM    = 10;

typedef RFC1867Parser<CGIRequestReader> RFC1867ParserImpl;
typedef MultipartMessageParser<CGIRequestReader> MessageParserImpl;

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << " <POST_FILE_PATH> <CONTENT_TYPE> <FILE_DIR_PATH> <FILE_DIGEST>" << endl;
}

void run_parse(apr_pool_t *pool, const char *post_file_path,
               const char *content_type, const char *file_digest,
               const char *file_dir_path, apr_size_t dump_level)
{
    apr_array_header_t *content_array;
    RequestReader::post_progress_t progress;
    ifstream stream(post_file_path, ios_base::binary);
    RFC1867ParserImpl::content_t *file_content;

    show_test_name("parse");

    File post_file(pool, post_file_path);
    CGIRequestReader reader(&progress, NULL, &stream);

    RFC1867ParserImpl parser(pool, reader, file_dir_path, MAX_TEXT_SIZE,
                             MAX_FILE_SIZE, MAX_ITEM_NUM);

    content_array = parser.parse(content_type,
                                 static_cast<apr_size_t>(post_file.get_size()));

    if (dump_level > 2) {
        RFC1867ParserImpl::dump_content_array(content_array);
    }

    file_content = MessageParserImpl::get_content(content_array,
                                                  FILE_CONTENT_NAME);
    if (file_digest != NULL) {
        if (strncmp(file_content->file.digest, file_digest,
                    strlen(file_digest)) != 0) {
            THROW(MESSAGE_BUG_FOUND);
        }
    }

    show_spacer();
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    const char *post_file_path;
    const char *content_type;
    const char *file_dir_path;
    const char *file_digest;
    apr_size_t dump_level;

    if (argc < 4) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    post_file_path = argv[1];
    content_type = argv[2];
    file_dir_path = argv[3];
    if (argc >= 5) {
        file_digest = argv[4];
    } else {
        file_digest = NULL;
    }
    if (argc >= 6) {
        dump_level = atoi(argv[5]);
    } else {
        dump_level = 0;
    }

    if (!File::is_exist(pool, post_file_path)) {
        THROW(MESSAGE_FILE_NOT_FOUND);
    }

    show_item("post_file_path", post_file_path);
    show_item("content_type", content_type);
    show_item("file_dir_path", file_dir_path);

    show_line();

    run_parse(pool, post_file_path, content_type, file_digest, file_dir_path,
              dump_level);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
