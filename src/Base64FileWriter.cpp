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
 * $Id: Base64FileWriter.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef TEMPLATE_INSTANTIATION
#include "Environment.h"
#endif

#include "Base64FileWriter.h"
#include "File.h"
#include "SourceInfo.h"

#ifndef TEMPLATE_INSTANTIATION
SOURCE_INFO_ADD("$Id: Base64FileWriter.cpp 2756 2007-12-11 10:57:59Z svn $");
#endif

#define BYTE(c) static_cast<apr_byte_t>(c)

template<class W> const apr_byte_t
Base64FileWriter<W>::ALPHABET[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/******************************************************************************
 * public メソッド
 *****************************************************************************/
template<class W>
Base64FileWriter<W>::Base64FileWriter(apr_pool_t *pool, apr_file_t *file,
                                      apr_size_t offset)
  : writer_impl_(pool, file, offset),
    decode_buffer_(0),
    decode_buffer_index_(0)
{
    memset(is_alphabet_, 0, sizeof(is_alphabet_));
    memset(alphabet_value_, 0, sizeof(alphabet_value_));

    for (apr_size_t i = 0; i < sizeof(ALPHABET); i++) {
        alphabet_value_[ALPHABET[i]]    = static_cast<char>(i);
        is_alphabet_[ALPHABET[i]]       = 1;
    }
}

/******************************************************************************
 * private メソッド
 *****************************************************************************/
template<class W>
apr_size_t Base64FileWriter<W>::write_impl(apr_byte_t *buffer,
                                           apr_size_t size)
{
    apr_size_t proc_size;
    apr_size_t write_size;
    apr_byte_t c;

    write_size = 0;
    for (proc_size = 0; proc_size < size; proc_size++) {
        c = buffer[proc_size];

        if (c == '=') {
            if (decode_buffer_index_ == 2) {
                buffer[write_size+0] = BYTE((decode_buffer_ >> 4) & 0xff);
                write_size += 1;
            } else if (decode_buffer_index_ == 3) {
                buffer[write_size+0] = BYTE((decode_buffer_ >> 10) & 0xff);
                buffer[write_size+1] = BYTE((decode_buffer_ >>  2) & 0xff);
                write_size += 2;
            }
            break;
        }
        if (!is_alphabet_[c]) {
            continue;
        }

        decode_buffer_ = (decode_buffer_ << 6) | alphabet_value_[c];
        decode_buffer_index_++;

        if (decode_buffer_index_ == 4) {
            buffer[write_size+0] = BYTE((decode_buffer_ >> 16) & 0xff);
            buffer[write_size+1] = BYTE((decode_buffer_ >>  8) & 0xff);
            buffer[write_size+2] = BYTE((decode_buffer_ >>  0) & 0xff);

            write_size += 3;

            decode_buffer_ = 0;
            decode_buffer_index_ = 0;
        }
    }

    writer_impl_.write(buffer, &write_size);

    return proc_size;
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_Base64FileWriter

#include "TestRunner.h"

static const apr_size_t BUFFER_SIZE     = 4096;

void show_usage(const char *prog_name)
{

    cerr << "Usage: " << prog_name << " <ENCODED_FILE> <DECODED_FILE>" << endl;
}

void run_copy(apr_pool_t *pool,
              const char *encoded_file_path, const char *decoded_file_path)
{
    volatile double start_time;
    volatile double end_time;
    apr_mmap_t *encoded_file_map;
    char *encoded_file_data;
    apr_size_t write_size;
    apr_size_t remain_size;

    show_test_name("copy");

    File encoded_file(pool, encoded_file_path);
    encoded_file.open(APR_READ|APR_BINARY);
    encoded_file_map = encoded_file.mmap();

    File decoded_file(pool, decoded_file_path);
    decoded_file.open(APR_CREATE|APR_TRUNCATE|APR_READ|APR_WRITE|APR_BINARY);
    Base64FileWriter<> decoded_file_writer(pool, decoded_file.get_handle());

    remain_size = static_cast<apr_size_t>(encoded_file.get_size()); // 4G の壁

    APR_PALLOC(encoded_file_data, char*, pool, encoded_file_map->size);
    memcpy(encoded_file_data, encoded_file_map->mm, encoded_file_map->size);

    start_time = get_usage_sec();
    while (remain_size > 0) {
        write_size = (remain_size > BUFFER_SIZE) ? BUFFER_SIZE : remain_size;

        decoded_file_writer.write(encoded_file_data, write_size);

        encoded_file_data += write_size;
        remain_size -= write_size;
    }
    decoded_file_writer.close();
    end_time = get_usage_sec();

    show_item("deocode copy", (end_time - start_time)*1000, " msec");
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    const char *encoded_file_path;
    const char *decoded_file_path;

    if (argc != 3) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    encoded_file_path = argv[1];
    decoded_file_path = argv[2];

    if (!File::is_exist(pool, encoded_file_path)) {
        THROW(MESSAGE_FILE_NOT_FOUND);
    }

    show_item("encoded_file_path", encoded_file_path);
    show_item("decoded_file_path", decoded_file_path);

    show_line();
    run_copy(pool, encoded_file_path, decoded_file_path);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
