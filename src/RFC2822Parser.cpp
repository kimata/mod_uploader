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
 * $Id: RFC2822Parser.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef TEMPLATE_INSTANTIATION
#include "Environment.h"
#endif

#include "RFC2822Parser.h"
#include "MultipartMessageParser.h"
#include "TemporaryFile.h"
#include "DirectoryCleaner.h"
#include "Auxiliary.h"
#include "Message.h"
#include "Macro.h"
#include "SourceInfo.h"

using namespace std;

#ifndef TEMPLATE_INSTANTIATION
SOURCE_INFO_ADD("$Id: RFC2822Parser.cpp 2756 2007-12-11 10:57:59Z svn $");
#endif

#ifdef DEBUG_RFC2822Parser
#define DUMP_INPUT_AND_THROW(message) M::dump_input(M::buffer_); THROW(message)
#else
#define DUMP_INPUT_AND_THROW(message) THROW(message)
#endif

#ifdef DEBUG_RFC2822Parser
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
RFC2822Parser<R, W>::MULTIPART_MIXED[]      = "multipart/mixed; ";

/******************************************************************************
 * public メソッド
 *****************************************************************************/
template<class R, class W>
RFC2822Parser<R, W>::RFC2822Parser(apr_pool_t *pool, PostReaderClass& reader,
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
apr_array_header_t *RFC2822Parser<R, W>::parse()
{
    apr_array_header_t *content_array;
    content_t content;

    TRACE_FUNC;

    // 古い一時ファイルを削除
    DirectoryCleaner::clean_old_files(M::pool_, M::file_dir_path_,
                                      M::READ_TIMEOUT_SEC);

    if (M::fill() == 0) {
        THROW(MESSAGE_RFC2822_CONTENT_SIZE_ZERO);
    }

    parse_header();

    content_array = apr_array_make(M::pool_,
                                   static_cast<int>(M::max_item_num_),
                                   static_cast<int>(sizeof(content_t)));

    M::buffer_.erase(M::skip_line(M::buffer_.get_data()) -
                     M::buffer_.get_data() - strlen(M::CR_LF));

    while (!M::is_end()) {
        if (static_cast<apr_size_t>(content_array->nelts) == M::max_item_num_) {
            THROW(MESSAGE_RFC2822_ITEM_COUNT_EXCEEDED);
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
void RFC2822Parser<R, W>::get_content(content_t *content)
{
    const char *content_type;
    const char *start;

    TRACE_FUNC;

    if ((M::fill() == 0) && (M::buffer_.get_size() == 0)) {
        DUMP_INPUT_AND_THROW(MESSAGE_RFC2822_FORMAT_INVALID);
    }

    if (!M::start_with(M::buffer_.get_data(), M::CR_LF)) {
        DUMP_INPUT_AND_THROW(MESSAGE_RFC2822_FORMAT_INVALID);
    }

    M::buffer_.erase(strlen(M::CR_LF));

    content_type = get_one_header();

    if (!M::start_with(content_type, M::CONTENT_TYPE)) {
        DUMP_INPUT_AND_THROW(MESSAGE_RFC2822_FORMAT_INVALID);
    }

    start = strnchr(content_type, strlen(content_type), M::DELIMITER);

    if (start == NULL) {
        content->file.mime = content_type + strlen(M::CONTENT_TYPE);
    } else {
        content->file.mime = AS_CONST_CHAR
            (apr_pstrmemdup(M::pool_, content_type+strlen(M::CONTENT_TYPE),
                            start-content_type-strlen(M::CONTENT_TYPE)));
        start = M::get_param(start, content_type+strlen(content_type),
			     M::NAME_PARAM, &(content->file.name));
    }

    content->name = "";

    if (start == NULL) { // テキスト
        skip_header();
        M::get_text_content(content);
    } else { // ファイル
        content->file.name = basename_ex(content->file.name);

        skip_header();
        M::get_file_content(content);
    }
}

template<class R, class W>
void RFC2822Parser<R, W>::skip_header()
{
    // 残りのヘッダを読み飛ばす
    while ((M::fill() != 0) || (M::buffer_.get_size() != 0)) {
        if (M::start_with(M::buffer_.get_data(), M::CR_LF)) {
            M::buffer_.erase(strlen(M::CR_LF));
            return;
        }

        M::buffer_.erase(M::skip_line(M::buffer_.get_data()) -
                         M::buffer_.get_data());
    }

    THROW(MESSAGE_RFC2822_HEADER_INVALID);
}

template<class R, class W>
const char *RFC2822Parser<R, W>::get_one_header()
{
    const char *header;
    const char *line_start;
    const char *line_end;

    header = "";
    line_start = M::buffer_.get_data();
    while (1) {
        line_end = M::skip_line(line_start);

        header = apr_pstrcat(M::pool_, header,
                             apr_pstrmemdup(M::pool_, line_start,
                                            line_end - line_start -
                                            strlen(M::CR_LF)),
                             NULL);
        line_start = line_end;

        if (*line_end != ' ') {
            break;
        }
    }

    M::buffer_.erase(line_end - M::buffer_.get_data());

    return header;
}

template<class R, class W>
void RFC2822Parser<R, W>::parse_header()
{
    const char *boundary;
    const char *content_type;
    const char *content_type_end;
    const char *start;

    content_type = NULL;
    content_type_end = NULL;
    while ((M::fill() != 0) || (M::buffer_.get_size() != 0)) {
        if (M::start_with(M::buffer_.get_data(), M::CR_LF)) {
            // ヘッダの終わり
            THROW(MESSAGE_RFC2822_HEADER_INVALID);
        }

        if (M::start_with(M::buffer_.get_data(), M::CONTENT_TYPE)) {
            if (content_type == NULL)  {
                content_type = M::buffer_.get_data();
                content_type_end = M::skip_line(M::buffer_.get_data()) -
                    strlen(M::CR_LF);
                break;
            }
        }

        start = M::skip_line(M::buffer_.get_data());
        M::buffer_.erase(start - M::buffer_.get_data());
    }

    if (content_type == NULL) {
        THROW(MESSAGE_RFC2822_FORMAT_INVALID);
    }

    start = M::skip(content_type + strlen(M::CONTENT_TYPE), MULTIPART_MIXED);
    if (M::get_param(start, content_type_end, M::BOUNDARY_PARAM,
		     &boundary) == NULL) {
        THROW(MESSAGE_RFC2822_CONTENT_TYPE_INVALID);
    }

    M::boundary_ = boundary;
    M::boundary_len_ = strlen(M::boundary_);
    M::barrier_len_ = M::boundary_len_ + strlen(M::CR_LF) +
        strlen(M::BOUNDARY_PREFIX);

    skip_header();
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_RFC2822Parser
#include <fstream>

#include "File.h"
#include "CGIRequestReader.h"

#define TEMPLATE_INSTANTIATION
#include "MultipartMessageParser.cpp"
#include "Base64FileWriter.cpp"

#include "TestRunner.h"

static const char FILE_CONTENT_NAME[]   = "file";
static const apr_size_t MAX_TEXT_SIZE   = 100 * 1024;
static const apr_size_t MAX_FILE_SIZE   = 100 * 1024 * 1024;
static const apr_size_t MAX_ITEM_NUM    = 10;

typedef RFC2822Parser<CGIRequestReader> RFC2822ParserImpl;

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << " <POST_FILE_PATH> <FILE_DIR_PATH> <FILE_DIGEST>" << endl;
}

void run_parse(apr_pool_t *pool, const char *post_file_path,
               const char *file_digest, const char *file_dir_path,
               apr_size_t dump_level)
{
    apr_array_header_t *content_array;
    RequestReader::post_progress_t progress;
    ifstream stream(post_file_path, ios_base::binary);
    RFC2822ParserImpl::content_t *file_content;

    show_test_name("parse");

    CGIRequestReader reader(&progress, NULL, &stream);

    RFC2822ParserImpl parser(pool, reader, file_dir_path, MAX_TEXT_SIZE,
                             MAX_FILE_SIZE, MAX_ITEM_NUM);

    content_array = parser.parse();

    if (dump_level > 2) {
        RFC2822ParserImpl::dump_content_array(content_array);
    }

    file_content = RFC2822ParserImpl::get_content(content_array,
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
    const char *file_dir_path;
    const char *file_digest;
    apr_size_t dump_level;

    if (argc < 3) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    post_file_path = argv[1];
    file_dir_path = argv[2];
    if (argc >= 4) {
        file_digest = argv[3];
    } else {
        file_digest = NULL;
    }
    if (argc >= 5) {
        dump_level = atoi(argv[4]);
    } else {
        dump_level = 0;
    }

    if (!File::is_exist(pool, post_file_path)) {
        THROW(MESSAGE_FILE_NOT_FOUND);
    }

    show_item("post_file_path", post_file_path);
    show_item("file_dir_path", file_dir_path);

    show_line();

    run_parse(pool, post_file_path, file_digest, file_dir_path, dump_level);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
