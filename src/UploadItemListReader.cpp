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
 * $Id: UploadItemListReader.cpp 2889 2008-05-10 16:36:45Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <memory>

#include "AtomicWrapper.h"
#include "apr_file_io.h"
#include "apr_mmap.h"
#include "apr_strings.h"

#include "UploadItemListReader.h"
#include "UploadItemList.h"
#include "UploadItemReader.h"
#include "ThumbnailList.h"
#include "TemporaryPool.h"
#include "File.h"
#include "Auxiliary.h"
#include "Message.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: UploadItemListReader.cpp 2889 2008-05-10 16:36:45Z svn $");

/******************************************************************************
 * public メソッド
 *****************************************************************************/
UploadItemList *UploadItemListReader::read(apr_pool_t *pool,
                                           const char *data_dir_path,
                                           const char *thumb_dir_path,
                                           apr_uint64_t max_file_size,
                                           apr_size_t max_list_size,
                                           apr_shm_t *shm,
                                           ThumbnailList *thumbnail_list,
                                           apr_time_t *mtime)
{
    UploadItemList *item_list;
    apr_dir_t *dir;
    apr_finfo_t sub_dir_info;
    char *sub_dir_path;
    TemporaryPool temp_pool(pool);

    *mtime = 0;

    UploadItemReader uitem_reader(temp_pool.get(),
                                  data_dir_path, NULL, thumb_dir_path);

    if (apr_dir_open(&dir, data_dir_path, temp_pool.get()) != APR_SUCCESS) {
        THROW(MESSAGE_UPLOAD_ITEM_DIR_OPEN_FAILED);
    }

    item_list = UploadItemList::get_instance(shm,
                                             max_file_size, max_list_size);

    while (apr_dir_read(&sub_dir_info,
                        APR_FINFO_NAME|APR_FINFO_TYPE|APR_FINFO_MTIME,
                        dir) == APR_SUCCESS) {
        if ((sub_dir_info.filetype != APR_DIR) ||
            !is_subdir_name_valid(sub_dir_info.name)) {
            continue;
        }
        if (sub_dir_info.mtime > *mtime) {
            *mtime = sub_dir_info.mtime;
        }

        if (apr_filepath_merge(&sub_dir_path, data_dir_path,
                               sub_dir_info.name, APR_FILEPATH_NOTABOVEROOT,
                               temp_pool.get()) != APR_SUCCESS) {
            THROW(MESSAGE_UPLOAD_ITEM_SUB_DIR_PATH_CREATION_FAILED);
        }

        get_subdir_threads(temp_pool.get(), sub_dir_path, &uitem_reader,
                           item_list, thumbnail_list);
    }
    apr_dir_close(dir);

    return item_list;
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void UploadItemListReader::get_subdir_threads(apr_pool_t *pool,
                                              const char *sub_dir_path,
                                              UploadItemReader *uitem_reader,
                                              UploadItemList *item_list,
                                              ThumbnailList *thumbnail_list)
{
    apr_dir_t *sub_dir;
    apr_finfo_t file_info;
    TemporaryPool temp_pool(pool);
    long buffer[PAD_DIV(sizeof(UploadItem), sizeof(long))];
    UploadItem *uitem = AS_UITEM(buffer);

    if (apr_dir_open(&sub_dir, sub_dir_path, temp_pool.get()) != APR_SUCCESS) {
        THROW(MESSAGE_UPLOAD_ITEM_SUB_DIR_OPEN_FAILED);
    }

    while (apr_dir_read(&file_info,
                        APR_FINFO_NAME|APR_FINFO_TYPE|APR_FINFO_SIZE,
                        sub_dir) == APR_SUCCESS) {
        if ((file_info.filetype != APR_REG) ||
            !is_file_name_valid(file_info.name)) {
            continue;
        }

        uitem_reader->read(atosize(file_info.name), uitem);
        item_list->add(uitem);
        if (uitem_reader->is_exist_thumbnail(uitem->get_id())) {
            thumbnail_list->add(uitem->get_id());
        }
    }
    apr_dir_close(sub_dir);
}

bool UploadItemListReader::is_subdir_name_valid(const char *subdir_name)
{
    apr_size_t i;

    for (i = 0; isxdigit(static_cast<unsigned char>(subdir_name[i])) != 0; i++) ;

    return (subdir_name[i] == '\0');
}

bool UploadItemListReader::is_file_name_valid(const char *file_name)
{
    apr_size_t i;

    for (i = 0; isdigit(static_cast<unsigned char>(file_name[i])) != 0; i++) ;

    return (file_name[i] == '\0');
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
