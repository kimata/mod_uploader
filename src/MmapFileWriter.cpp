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
 * $Id: MmapFileWriter.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef TEMPLATE_INSTANTIATION
#include "Environment.h"
#endif

#if defined(HAVE_UNISTD_H) && !defined(WIN32)
#include <unistd.h>
#endif

#include "MmapFileWriter.h"
#include "File.h"
#include "Macro.h"
#include "SourceInfo.h"

#ifndef TEMPLATE_INSTANTIATION
SOURCE_INFO_ADD("$Id: MmapFileWriter.cpp 2756 2007-12-11 10:57:59Z svn $");
#endif

#if defined(HAVE_GETPAGESIZE) && !defined(WIN32)
const apr_size_t MmapFileWriter::PAGE_SIZE      = getpagesize();
#else
const apr_size_t MmapFileWriter::PAGE_SIZE      = SYS_PAGE_SIZE;
#endif
const apr_size_t MmapFileWriter::BLOCK_SIZE     = ((SYS_MMAP_BLOCK_SIZE-1)/PAGE_SIZE + 1) * PAGE_SIZE;

/******************************************************************************
 * public メソッド
 *****************************************************************************/
MmapFileWriter::MmapFileWriter(apr_pool_t *pool, apr_file_t *file, apr_size_t offset)
    : FileWriter(pool, file),
      file_map_(NULL),
      block_index_(0),

      block_offset_(0)
{
    expand();
    block_offset_ = offset;
}

void MmapFileWriter::close()
{
    if (file_->is_closed()) {
        return;
    }

    file_->trunc(static_cast<apr_size_t>(BLOCK_SIZE * (block_index_ - 1) +
                                         block_offset_));
    FileWriter::close();
}

MmapFileWriter::~MmapFileWriter()
{
    try {
        close();
    } catch(const char *) {
        // 無視
    }
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
apr_size_t MmapFileWriter::write_impl(const void *buffer, apr_size_t size)
{
    const char *data;
    apr_size_t remain_size;
    apr_size_t write_size;

    remain_size = size;
    data = AS_CONST_CHAR(buffer);

    while (remain_size > 0) {
        write_size
            = (remain_size > BLOCK_SIZE) ? BLOCK_SIZE : remain_size;

        if (write_size > (BLOCK_SIZE-block_offset_)) {
            if (block_offset_ != BLOCK_SIZE) {
                write_size = BLOCK_SIZE - block_offset_;

                memcpy(AS_CHAR(file_map_->mm)+block_offset_, data, write_size);
                data += write_size;
                remain_size -= write_size;
            }
            expand();

            continue;
        }

        memcpy(AS_CHAR(file_map_->mm)+block_offset_, data, write_size);

        data += write_size;
        block_offset_ += write_size;
        remain_size -= write_size;
    }

    return size;
}

void MmapFileWriter::expand()
{
    apr_uint64_t pseudo_size;

    block_index_++;
    pseudo_size = BLOCK_SIZE * block_index_;

    file_->expand(pseudo_size);
    file_map_ = file_->mmap(pseudo_size - BLOCK_SIZE, BLOCK_SIZE,
                            APR_MMAP_READ|APR_MMAP_WRITE);

#if defined(HAVE_MADVISE) && !defined(WIN32)
    // シーケンシャルアクセスしかしない
    madvise(file_map_->mm, file_map_->size, MADV_SEQUENTIAL);
#endif

    block_offset_ = 0;
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_MmapFileWriter
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
    MmapFileWriter to_file_writer(pool, to_file.get_handle());

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
