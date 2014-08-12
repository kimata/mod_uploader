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
 * $Id: MultipartMessageParser.cpp 2872 2008-04-27 14:18:02Z svn $
 *****************************************************************************/

#ifndef TEMPLATE_INSTANTIATION
#include "Environment.h"
#endif

#include "MultipartMessageParser.h"
#include "TemporaryFile.h"
#include "Auxiliary.h"
#include "Message.h"
#include "Macro.h"

using namespace std;

#undef DUMP_INPUT_AND_THROW
#ifdef DEBUG_MultipartMessageParser
#define DUMP_INPUT_AND_THROW(message) dump_input(buffer_); THROW(message)
#else
#define DUMP_INPUT_AND_THROW(message) THROW(message)
#endif

#undef DUMP_TOKEN_AND_THROW
#undef TRACE_FUNC
#ifdef DEBUG_MultipartMessageParser
extern bool parser_is_trace;
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

template<class R, class W> const apr_size_t
MultipartMessageParser<R, W>::READ_BLOCK_SIZE       = REQ_READ_BLOCK_SIZE;
template<class R, class W> const apr_size_t
MultipartMessageParser<R, W>::READ_TIMEOUT_SEC      = REQ_READ_TIMEOUT_SEC;
template<class R, class W> const char
MultipartMessageParser<R, W>::CR_LF[]               = "\r\n";
template<class R, class W> const char
MultipartMessageParser<R, W>::CR_LF_CR_LF[]         = "\r\n\r\n";
template<class R, class W> const char
MultipartMessageParser<R, W>::CONTENT_TYPE[]        = "Content-Type: ";
template<class R, class W> const char
MultipartMessageParser<R, W>::CONTENT_DISPOSITION[] = "Content-Disposition: ";
template<class R, class W> const char
MultipartMessageParser<R, W>::BOUNDARY_PARAM[]      = "boundary";
template<class R, class W> const char
MultipartMessageParser<R, W>::BOUNDARY_PREFIX[]     = "--";
template<class R, class W> const char
MultipartMessageParser<R, W>::ASSIGN                = '=';
template<class R, class W> const char
MultipartMessageParser<R, W>::QUOTE                 = '"';
template<class R, class W> const char
MultipartMessageParser<R, W>::DELIMITER             = ';';
template<class R, class W> const char
MultipartMessageParser<R, W>::NAME_PARAM[]          = "name";
template<class R, class W> const char
MultipartMessageParser<R, W>::FILE_NAME_TEMPLATE[]  = "post";

/******************************************************************************
 * public メソッド
 *****************************************************************************/
template<class R, class W>
MultipartMessageParser<R, W>::MultipartMessageParser(apr_pool_t *pool,
                                                     PostReaderClass& reader,
                                                     const char *file_dir_path,
                                                     apr_size_t max_text_size,
                                                     apr_uint64_t max_file_size,
                                                     apr_size_t max_item_num,
                                                     apr_size_t file_offset)
  : pool_(pool),
    reader_(reader),
    buffer_(READ_BLOCK_SIZE * 2), // 2 以上じゃないとまずい
    boundary_(NULL),
    boundary_len_(0),
    barrier_len_(0),
    file_dir_path_(file_dir_path),
    max_text_size_(max_text_size),
    max_file_size_(max_file_size),
    max_item_num_(max_item_num),
    file_offset_(file_offset)
{

}

template<class R, class W>
MultipartMessageParser<R, W>::~MultipartMessageParser()
{

}

template<class R, class W>
typename MultipartMessageParser<R, W>::content_t *
MultipartMessageParser<R, W>::get_content(apr_array_header_t *content_array,
                                          const char *name)
{
    content_t *contents;

    contents = AS_CONTENT(AVOID_ALIGN_WARN content_array->elts);
    for (int i = 0; i < content_array->nelts; i++) {
        if (strncmp((contents + i)->name, name, strlen(name)) == 0) {
            return contents + i;
        }
    }

    return NULL;
}

template<class R, class W>
typename MultipartMessageParser<R, W>::content_t *
MultipartMessageParser<R, W>::get_text_content(apr_array_header_t *content_array,
                                               apr_size_t index)
{
    content_t *contents;
    apr_size_t count;

    contents = AS_CONTENT(AVOID_ALIGN_WARN content_array->elts);
    count = 0;
    for (int i = 0; i < content_array->nelts; i++) {
        if ((contents + i)->type == TEXT) {
            if (count++ == index) {
                return (contents + i);
            }
        }
    }

    return NULL;
}

template<class R, class W>
typename MultipartMessageParser<R, W>::content_t *
MultipartMessageParser<R, W>::get_file_content(apr_array_header_t *content_array,
                                               apr_size_t index)
{
    content_t *contents;
    apr_size_t count;

    contents = AS_CONTENT(AVOID_ALIGN_WARN content_array->elts);
    count = 0;
    for (int i = 0; i < content_array->nelts; i++) {
        if ((contents + i)->type == FILE) {
            if (count++ == index) {
                return (contents + i);
            }
        }
    }

    return NULL;
}

template<class R, class W>
void MultipartMessageParser<R, W>::dump_content_array(apr_array_header_t *content_array)
{
    content_t *contents;

    cout << "****************************************" << endl;

    contents = AS_CONTENT(AVOID_ALIGN_WARN content_array->elts);
    for (int i = 0; i < content_array->nelts; i++) {
        dump_content(contents + i);
        cout << "****************************************" << endl;
    }
}


/******************************************************************************
 * protected メソッド
 *****************************************************************************/
template<class R, class W>
void MultipartMessageParser<R, W>::get_text_content(content_t *content)
{
    char *text;
    char *old_text;
    apr_size_t text_length;
    const char *end;
    apr_size_t read_size;
    apr_size_t tail_size;

    TRACE_FUNC;

    content->type = TEXT;

    text = NULL;
    text_length = 0;

    end = AS_CONST_CHAR(memmem(buffer_.get_data(), buffer_.get_size(),
                               boundary_, boundary_len_));

    if (end != NULL) {
        content->text = apr_pstrmemdup(pool_,
                                       buffer_.get_data(),
                                       end - buffer_.get_data() -
                                       strlen(CR_LF) -
                                       strlen(BOUNDARY_PREFIX));
        end += boundary_len_;
        buffer_.erase(end - buffer_.get_data());

        return;
    }

    try {
        MALLOC(text, char *, sizeof(char), buffer_.get_size()); // 多めに確保
        write_text(&text, &text_length, buffer_, barrier_len_);

        // このループは通常数回で抜けるはず
        while (UNLIKELY(true)) {
            if (text_length > max_text_size_) {
                DUMP_INPUT_AND_THROW(MESSAGE_RFC2822_TEXT_SIZE_TOO_LARGE);
            }

            read_size = read();

            end = AS_CONST_CHAR(memmem(buffer_.get_data(),
                                       buffer_.get_size(), boundary_,
                                       boundary_len_));
            if (end != NULL) { // バッファ中にバウンダリが見つかった
                break;
            } else if (read_size == 0) { // バウンダリが見つからないまま入力終了
                DUMP_INPUT_AND_THROW(MESSAGE_RFC2822_FORMAT_INVALID);
            }

            old_text = text;
            REALLOC(text, char *, sizeof(char),
                    text_length + buffer_.get_size());
            write_text(&text, &text_length, buffer_, barrier_len_);
        }

        old_text = text;
        tail_size = end -
            strlen(CR_LF) - strlen(BOUNDARY_PREFIX) -
            buffer_.get_data();

        APR_PALLOC(text, char *, pool_, text_length + tail_size + 1);

        memcpy(text, old_text, text_length);
        memcpy(text + text_length, buffer_.get_data(), tail_size);
        *(text + text_length + tail_size) = '\0';

        end += boundary_len_;
        buffer_.erase(end - buffer_.get_data());

        free(old_text);

        content->text = text;
    } catch(const char *) {
        if (text != NULL) {
            free(text);
        }
        throw;
    }
}

template<class R, class W>
void MultipartMessageParser<R, W>::get_file_content(content_t *content)
{
    const char *end;
    apr_size_t read_size;
    apr_size_t write_size;

    TRACE_FUNC;

    content->type = FILE;

    TemporaryFile temp_file(pool_,
                            apr_pstrcat(pool_, file_dir_path_,
                                        "/", FILE_NAME_TEMPLATE, NULL),
                            false);
    temp_file.open(FileWriterClass::OPEN_FLAG);

    FileWriterClass writer(pool_, temp_file.get_handle(), file_offset_);
    MessageDigest5 digest;

    end = AS_CONST_CHAR(memmem(buffer_.get_data(), buffer_.get_size(),
                               boundary_, boundary_len_));

    if (end == NULL) { // まだバッファ中にバウンダリがない
        while (true) {
            if (UNLIKELY(writer.get_write_size() > max_file_size_)) {
                DUMP_INPUT_AND_THROW(MESSAGE_RFC1867_FILE_SIZE_TOO_LARGE);
            }

            // バウンダリが途中までバッファ中に含まれている可能性がある
            // のでそれを考慮して書き出すサイズを計算
            if (buffer_.get_size() < barrier_len_) {
                DUMP_INPUT_AND_THROW(MESSAGE_RFC1867_FORMAT_INVALID);
            }

            write_size = buffer_.get_size() - barrier_len_;
            write_file(writer, digest, buffer_, write_size);

            read_size = fill();
            end = AS_CONST_CHAR(memmem(buffer_.get_data(),
                                       buffer_.get_size(), boundary_,
                                       boundary_len_));
            if (UNLIKELY(end != NULL)) { // バッファ中にバウンダリが見つかった
                break;
            } else if (UNLIKELY(read_size == 0)) { // バウンダリが見つからないまま入力終了
                DUMP_INPUT_AND_THROW(MESSAGE_RFC1867_FORMAT_INVALID);
            }
        }
    }

    if (end > buffer_.get_data()+strlen(CR_LF)+strlen(BOUNDARY_PREFIX)) {
        write_size = end - buffer_.get_data() -
            strlen(CR_LF) - strlen(BOUNDARY_PREFIX);

        write_file(writer, digest, buffer_, write_size);
    } else {
        write_size = 0;
    }

    end = end - write_size + boundary_len_;
    buffer_.erase(end - buffer_.get_data());

    writer.close();
    digest.finish();

    content->file.size = writer.get_write_size();
    content->file.digest = apr_pstrdup(pool_, digest.c_str());
    content->file.temp_path = temp_file.get_temp_path();
}

template<class R, class W>
bool MultipartMessageParser<R, W>::is_end()
{
    TRACE_FUNC;

    fill();

    return start_with(buffer_.get_data(), BOUNDARY_PREFIX);
}

template<class R, class W>
apr_size_t MultipartMessageParser<R, W>::fill()
{
    apr_size_t size;
    apr_size_t read_size = 0;

    TRACE_FUNC;

    while (buffer_.get_size() < READ_BLOCK_SIZE) {
        if ((size = read()) == 0) {
            break;
        }

        read_size += size;
    }

    return read_size;
}

template<class R, class W>
apr_size_t MultipartMessageParser<R, W>::read(apr_size_t size)
{
    apr_size_t read_size = 0;

    TRACE_FUNC;

    reader_.read(buffer_.get_data_end(), size, &read_size);
    buffer_.add_size(read_size);

    return read_size;
}

template<class R, class W>
const char *MultipartMessageParser<R, W>::get_param(const char *input_start,
                                           const char *input_end,
                                           const char *name,
                                           const char **value)
{
    const char *end;

    TRACE_FUNC;

    while (isspace(*input_start & 0xff) || (*input_start == DELIMITER)) {
        input_start++;
    }

    if (!start_with(input_start, name)) {
        *value = NULL;

        return NULL;
    }

    input_start += strlen(name);
    if (*(input_start++) != ASSIGN) {
        *value = NULL;

        return NULL;
    }

    if (*input_start == QUOTE) {
        input_start++;
        end = strnchr(input_start, input_end-input_start, QUOTE);
        if (end == NULL) {
            DUMP_INPUT_AND_THROW(MESSAGE_RFC1867_FORMAT_INVALID);
        }

        *value = static_cast<const char *>(apr_pstrmemdup(pool_, input_start,
                                                          end-input_start));
        end++;
    } else {
        end = input_start + 1;
        while ((end < input_end) && !isspace(*end & 0xff)) {
            end++;
        }
        *value = static_cast<const char *>(apr_pstrmemdup(pool_, input_start,
                                                          end-input_start));
    }

    return end;
}

template<class R, class W>
const char *MultipartMessageParser<R, W>::skip(const char *input_start,
                                      const char *pattern, bool is_must)
{
    TRACE_FUNC;

    if (!start_with(input_start, pattern)) {
        if (is_must) {
            DUMP_INPUT_AND_THROW(MESSAGE_RFC1867_FORMAT_INVALID);
        }

        return input_start;
    }

    return input_start + strlen(pattern);
}

template<class R, class W>
const char *MultipartMessageParser<R, W>::skip_line(const char *input_start)
{
    const char *start = strstr(input_start, CR_LF);

    TRACE_FUNC;

    if (start == NULL) {
        DUMP_INPUT_AND_THROW(MESSAGE_RFC1867_FORMAT_INVALID);
    }

    return start + strlen(CR_LF);
}

template<class R, class W>
const char *MultipartMessageParser<R, W>::skip_header(const char *input_start)
{
    const char *start = strstr(input_start, CR_LF_CR_LF);

    TRACE_FUNC;

    if (start == NULL) {
        DUMP_INPUT_AND_THROW(MESSAGE_RFC1867_FORMAT_INVALID);
    }

    return start + strlen(CR_LF_CR_LF);
}

template<class R, class W>
void MultipartMessageParser<R, W>::write_text(char **text, apr_size_t *text_length,
                                     MultipartMessageParserBuffer& buffer,
                                     apr_size_t barrier_len)
{
    TRACE_FUNC;

    if (buffer.get_size() < barrier_len) {
        return;
    }

    memcpy(*text + *text_length, buffer.get_data(),
           buffer.get_size() - barrier_len);
    *text_length += buffer.get_size() - barrier_len;
    buffer.erase(buffer.get_size() - barrier_len);
}

template<class R, class W>
void MultipartMessageParser<R, W>::write_file(FileWriterClass& writer,
                                     MessageDigest5& digest,
                                     MultipartMessageParserBuffer& buffer,
                                     apr_size_t size)
{
    apr_size_t write_size;

    TRACE_FUNC;

    write_size = size;

    writer.write(buffer.get_data(), &write_size);
    digest.update(AS_BYTE(buffer.get_data()), write_size);
    buffer.erase(size);
}

template<class R, class W>
bool MultipartMessageParser<R, W>::start_with(const char *str, const char *pattern,
                                     apr_size_t pattern_length)
{
    TRACE_FUNC;

    return strncmp(str, pattern, pattern_length) == 0;
}

template<class R, class W>
void MultipartMessageParser<R, W>::dump_content(content_t *content)
{
    cout << "name           : " << content->name << endl;

    if (content->type == TEXT) {
        cout << "type           : TEXT" << endl;
        cout << "value          : " << content->text << endl;
    } else if (content->type == FILE) {
        cout << "type           : FILE" << endl;
        cout << "file.name      : " << content->file.name << endl;
        cout << "file.temp_path : " << content->file.temp_path << endl;
        cout << "file.size      : " << content->file.size << endl;
        cout << "file.mime      : " << content->file.mime << endl;
        cout << "file.digest    : " << content->file.digest << endl;
    } else {
        cout << "type           : UNKNOWN" << endl;
    }
}

template<class R, class W>
void MultipartMessageParser<R, W>::dump_input(MultipartMessageParserBuffer& buffer)
{
    *(buffer.get_data() + buffer.get_size() - 1) = '\0';

    cerr << "INPUT:" << endl;
    cerr << buffer.get_data() << endl;
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
