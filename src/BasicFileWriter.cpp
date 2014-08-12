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
 * $Id: BasicFileWriter.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef TEMPLATE_INSTANTIATION
#include "Environment.h"
#endif

#include "BasicFileWriter.h"
#include "File.h"
#include "SourceInfo.h"

#ifndef TEMPLATE_INSTANTIATION
SOURCE_INFO_ADD("$Id: BasicFileWriter.cpp 2756 2007-12-11 10:57:59Z svn $");
#endif

/******************************************************************************
 * public メソッド
 *****************************************************************************/
BasicFileWriter::BasicFileWriter(apr_pool_t *pool, apr_file_t *file,
                                 apr_size_t offset)
    : FileWriter(pool, file)
{
    file_->seek(offset);
}

/******************************************************************************
 * private メソッド
 *****************************************************************************/
apr_size_t BasicFileWriter::write_impl(const void *buffer, apr_size_t size)
{
    file_->write(buffer, &size);

    return size;
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_BasicFileWriter
#include "TestRunner.h"

static const apr_size_t BUFFER_SIZE     = 4096;

void show_usage(const char *prog_name)
{

    cerr << "Usage: " << prog_name << " <FROM> <TO>" << endl;
}

void run_copy(apr_pool_t *pool,
              const char *from_file_path, const char *to_file_path)
{
    volatile double start_time;
    volatile double end_time;
    apr_mmap_t *from_file_map;
    const char *from_file_data;
    apr_size_t write_size;
    apr_size_t remain_size;

    show_test_name("copy");

    File from_file(pool, from_file_path);
    from_file.open(APR_READ|APR_BINARY);
    from_file_map = from_file.mmap();

    File to_file(pool, to_file_path);
    to_file.open(APR_CREATE|APR_TRUNCATE|APR_READ|APR_WRITE|APR_BINARY);
    BasicFileWriter to_file_writer(pool, to_file.get_handle());

    remain_size = static_cast<apr_size_t>(from_file.get_size()); // 4G の壁
    from_file_data = AS_CONST_CHAR(from_file_map->mm);
    start_time = get_usage_sec();
    while (remain_size > 0) {
        write_size = (remain_size > BUFFER_SIZE) ? BUFFER_SIZE : remain_size;

        to_file_writer.write(from_file_data, write_size);

        from_file_data += write_size;
        remain_size -= write_size;
    }
    to_file_writer.close();
    end_time = get_usage_sec();

    if (diff(pool, from_file_path, to_file_path) != 0) {
        THROW(MESSAGE_FILE_DIFFER);
    }

    show_item("copy", (end_time - start_time)*1000, " msec");
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    const char *from_file_path;
    const char *to_file_path;

    if (argc != 3) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    from_file_path = argv[1];
    to_file_path = argv[2];

    if (!File::is_exist(pool, from_file_path)) {
        THROW(MESSAGE_FILE_NOT_FOUND);
    }

    show_item("from_file_path", from_file_path);
    show_item("to_file_path", to_file_path);

    show_line();
    run_copy(pool, from_file_path, to_file_path);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
