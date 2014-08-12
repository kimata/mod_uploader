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
 * $Id: UploadItemWriter.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#ifdef MAKE_THUMBNAIL
#ifndef WIN32
#define USE_FORK
#endif
#endif

#ifdef USE_FORK
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "UploadItemWriter.h"
#include "UploadItem.h"
#include "TemporaryPool.h"
#include "TemporaryFile.h"
#include "ImageFile.h"
#include "Message.h"
#include "Macro.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: UploadItemWriter.cpp 2756 2007-12-11 10:57:59Z svn $");

/******************************************************************************
 * public メソッド
 *****************************************************************************/
UploadItemWriter::UploadItemWriter(apr_pool_t *pool, const char *data_dir_path,
                                   const char *file_dir_path,
                                   const char *thumb_dir_path)
  : UploadItemIO(pool, data_dir_path, file_dir_path, thumb_dir_path)
{

}

bool UploadItemWriter::write(UploadItem *uitem, const char *temp_file_path)
{
    TemporaryPool temp_pool(pool_);
    bool is_thumbnail_created;

    is_thumbnail_created = false;
    try {
        prepare_subdir(temp_pool.get(), uitem->get_id());

        write_data(temp_pool.get(), uitem);
        write_file(temp_pool.get(), uitem, temp_file_path);

#ifdef MAKE_THUMBNAIL
        if (uitem->get_file_size() < VIW_THUMBNAIL_SIZE_LIMIT) {
            is_thumbnail_created = write_thumb(temp_pool.get(), uitem);
        }
#endif
    } catch(const char *) {
        remove(temp_pool.get(), uitem);
        throw;
    }

    return is_thumbnail_created;
}

void UploadItemWriter::write_data(apr_pool_t *pool,
                                  UploadItem::header_t *header)
{
    write_data(pool, AS_UITEM(header));
    header->is_need_sync = false;
}

void UploadItemWriter::remove(apr_pool_t *pool, UploadItem *uitem)
{
    apr_file_remove(get_data_path(pool, uitem->get_id()), pool);
    apr_file_remove(get_file_path(pool, uitem), pool);
    apr_file_remove(get_thumb_path(pool, uitem->get_id()), pool);
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void UploadItemWriter::write_data(apr_pool_t *pool, UploadItem *uitem)
{
    const char *uitem_data_path;
    apr_mmap_t *uitem_data_map;

    uitem_data_path = get_data_path(pool, uitem->get_id());

    TemporaryFile uitem_data(pool, uitem_data_path);

    uitem_data.open(APR_WRITE);
    uitem_data.expand(sizeof(UploadItem));
    uitem_data_map = uitem_data.mmap(0, sizeof(UploadItem),
                                     APR_MMAP_READ|APR_MMAP_WRITE);

    memcpy(uitem_data_map->mm, uitem, sizeof(UploadItem));

    uitem_data.commit();
}

void UploadItemWriter::write_file(apr_pool_t *pool, UploadItem *uitem,
                                  const char *temp_file_path)
{
    if (apr_file_rename(temp_file_path, get_file_path(pool_, uitem),
                        pool) != APR_SUCCESS) {
        THROW(MESSAGE_FILE_RENAME_FAILED);
    }
}

#ifdef MAKE_THUMBNAIL
bool UploadItemWriter::write_thumb(apr_pool_t *pool, UploadItem *uitem)
{
#ifdef USE_FORK
    static const int TIMEOUT_SEC    = 100;
    static const int NICE_VALUE     = 20;

    pid_t pid;
    int status;

    switch (pid = fork()) {
    case 0: // 子プロセス．
        apr_pool_create(&pool, NULL);

        // 念のため nice 値を設定
        nice(NICE_VALUE);
        alarm(TIMEOUT_SEC);

        exit(write_thumb_impl(pool, uitem) ? EXIT_SUCCESS : EXIT_FAILURE);
    case -1: // エラー．
        return false;
    default: // 親プロセス
        waitpid(pid, &status, 0);

        return WIFEXITED(status) && (WEXITSTATUS(status) == EXIT_SUCCESS);
    }
#else
    return write_thumb_impl(pool, uitem);
#endif
}

bool UploadItemWriter::write_thumb_impl(apr_pool_t *pool, UploadItem *uitem)
{
    apr_size_t thumb_width;
    apr_size_t thumb_height;

    try {
        ImageFile image(pool, get_file_path(pool_, uitem));

        get_thumbnail_size(&image, &thumb_width, &thumb_height);

        image.create_thumbnail(get_thumb_path(pool, uitem->get_id()),
                               thumb_width, thumb_height);

        return true;
    } catch(const char *) {
        return false;
    }
}

void UploadItemWriter::get_thumbnail_size(ImageFile *image_file,
                                          apr_size_t *width,
                                          apr_size_t *height)
{
    static const apr_size_t ASPECT_FACTOR   = 1024;
    static const apr_size_t THUMBAIL_ASPECT =
        VIW_THUMBNAIL_WIDTH_LIMIT*ASPECT_FACTOR/VIW_THUMBNAIL_HEIGHT_LIMIT;

    *width = image_file->get_width();
    *height = image_file->get_height();

    if (((*width) == 0) || ((*height) == 0)) {
        THROW(MESSAGE_POST_IMAGE_SIZE_INVALID);
    }

    if (((*width)*ASPECT_FACTOR/(*height)) > THUMBAIL_ASPECT) {
        // 横長の場合
        if ((*width) > VIW_THUMBNAIL_WIDTH_LIMIT) {
            *height = (*height) * VIW_THUMBNAIL_WIDTH_LIMIT / (*width);
            *width = VIW_THUMBNAIL_WIDTH_LIMIT;
        }
    } else {
        // 縦長の場合
        if ((*height) > VIW_THUMBNAIL_HEIGHT_LIMIT) {
            *width = (*width) * VIW_THUMBNAIL_HEIGHT_LIMIT / (*height);
            *height = VIW_THUMBNAIL_HEIGHT_LIMIT;
        }
    }

    if (((*width) == 0) || ((*height) == 0)) {
        THROW(MESSAGE_POST_IMAGE_SIZE_INVALID);
    }
}
#endif


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_UploadItemWriter
#include "TestRunner.h"

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name;
    cerr << " <DATA_DIR_PATH> <FILE_DIR_PATH> <THUMB_DIR_PATH> <FILE_PATH>";
    cerr << endl;
}

static apr_size_t create_item_id()
{
    apr_size_t item_id;

    // 0 以外の適等な値にする
    item_id = static_cast<apr_size_t>(apr_time_now() % 10000) + 1;

    return item_id;
}

static apr_size_t create_item_index()
{
    apr_size_t item_index;

    // 適等な値にする
    item_index = static_cast<apr_size_t>(apr_time_now() % 10000);

    return item_index;
}

UploadItem *create_item(apr_pool_t *pool, const char *file_path)
{
    const char *file_name;
    const char *file_mime;
    File file(pool, file_path);

    file_name = basename_ex(file_path);
    file_mime = get_image_mime(file_name);

    return UploadItem::get_instance(pool,
                                    create_item_id(), create_item_index(),
                                    file.get_size(), apr_time_now(),
                                    MESSAGE_TEST_IP_ADDRESS,
                                    file_name, file_mime,
                                    MESSAGE_TEST_FILE_DIGEST,
                                    MESSAGE_TEST_REMOVE_PASS,
                                    MESSAGE_TEST_DOWNLOAD_PASS,
                                    MESSAGE_TEST_COMMENT);
}

const char *temp_file(apr_pool_t *pool, const char *file_path)
{
    TemporaryFile temp_file(pool, file_path, false);

    if (apr_file_copy(file_path, temp_file.get_temp_path(),
                      APR_FILE_SOURCE_PERMS, pool) != APR_SUCCESS) {
        THROW(MESSAGE_FILE_COPY_FAILED);
    }

    return temp_file.get_temp_path();
}

void run_write(apr_pool_t *pool, const char *data_dir_path,
               const char *file_dir_path, const char *thumb_dir_path,
               UploadItem *uitem, const char *file_path)
{
    show_test_name("write");

    UploadItemWriter writer(pool,
                            data_dir_path, file_dir_path, thumb_dir_path);

    writer.write(uitem, temp_file(pool, file_path));

    show_item("id", uitem->get_id());
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    const char *data_dir_path;
    const char *file_dir_path;
    const char *thumb_dir_path;
    const char *file_path;
    UploadItem *uitem;

    if (argc != 5) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    data_dir_path   = argv[1];
    file_dir_path   = argv[2];
    thumb_dir_path  = argv[3];
    file_path       = argv[4];

    uitem = create_item(pool, file_path);

    show_item("data_dir_path", data_dir_path);
    show_item("file_dir_path", file_dir_path);
    show_item("thumb_dir_path", thumb_dir_path);
    show_item("file_path", file_path);
    show_line();

    run_write(pool, data_dir_path, file_dir_path, thumb_dir_path,
              uitem, file_path);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
