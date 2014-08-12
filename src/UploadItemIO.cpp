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
 * $Id: UploadItemIO.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#include "apr_file_info.h"
#include "apr_file_io.h"
#include "apr_strings.h"

#include "UploadItemIO.h"
#include "UploadItem.h"
#include "Message.h"
#include "Macro.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: UploadItemIO.cpp 2756 2007-12-11 10:57:59Z svn $");

const char UploadItemIO::FILE_EXT_SEPARATOR[]   = ".";

/******************************************************************************
 * public メソッド
 *****************************************************************************/
UploadItemIO::UploadItemIO(apr_pool_t *pool, const char *data_dir_path,
                           const char *file_dir_path,
                           const char *thumb_dir_path)
  : pool_(pool),
    data_dir_path_(data_dir_path),
    file_dir_path_(file_dir_path),
    thumb_dir_path_(thumb_dir_path)
{

}

UploadItemIO::~UploadItemIO()
{

}

const char *UploadItemIO::get_file_path(apr_pool_t *pool,
                                        const char *file_dir_path,
                                        UploadItem *uitem)
{
    return get_path
        (pool, file_dir_path, uitem->get_id(),
         apr_pstrcat(pool,
                     apr_itoa(pool, static_cast<int>(uitem->get_id())),
                     FILE_EXT_SEPARATOR, uitem->get_file_ext(),
                     NULL));
}

const char *UploadItemIO::get_thumb_path(apr_pool_t *pool,
                                         const char *thumb_dir_path,
                                         apr_size_t item_id)
{
    return get_path(pool, thumb_dir_path, item_id,
                    apr_pstrcat(pool,
                                apr_itoa(pool, static_cast<int>(item_id)),
                                FILE_EXT_SEPARATOR, VIW_THUMBNAIL_EXT,
                                NULL));
}


/******************************************************************************
 * protected メソッド
 *****************************************************************************/
const char *UploadItemIO::get_data_path(apr_pool_t *pool,
                                       apr_size_t item_id) const
{
    return get_path(pool, data_dir_path_, item_id,
                    apr_itoa(pool, static_cast<int>(item_id)));
}

const char *UploadItemIO::get_file_path(apr_pool_t *pool,
                                        UploadItem *uitem) const
{
    return get_file_path(pool, file_dir_path_, uitem);
}

const char *UploadItemIO::get_thumb_path(apr_pool_t *pool,
                                         apr_size_t item_id) const
{
    return get_thumb_path(pool, thumb_dir_path_, item_id);
}

void UploadItemIO::prepare_subdir(apr_pool_t *pool, apr_size_t item_id) const
{
    prepare_subdir(pool, data_dir_path_, item_id);
    prepare_subdir(pool, file_dir_path_, item_id);
    prepare_subdir(pool, thumb_dir_path_, item_id);
}

const char *UploadItemIO::get_path(apr_pool_t *pool, const char *dir_path,
                                   apr_size_t item_id, const char *file_name)
{
    char *file_path;

    if (apr_filepath_merge(&file_path,
                           get_sub_dir_path(pool, dir_path, item_id),
                           file_name,
                           APR_FILEPATH_NOTABOVEROOT, pool) != APR_SUCCESS) {
        THROW(MESSAGE_UPLOAD_ITEM_FILE_PATH_CREATION_FAILED);
    }

    return file_path;
}

const char *UploadItemIO::get_sub_dir_path(apr_pool_t *pool,
                                          const char *dir_path,
                                          apr_size_t item_id)
{
    static const char HEX_CHAR[]    = "0123456789abcdef";

    char *sub_dir_path;
    char *sub_dir_name;

    APR_PALLOC(sub_dir_name, char *, pool, 3);
    sub_dir_name[0] = HEX_CHAR[(item_id >> 4) & 0xf];
    sub_dir_name[1] = HEX_CHAR[(item_id     ) & 0xf];
    sub_dir_name[2] = '\0';

    if (apr_filepath_merge(&sub_dir_path, dir_path, sub_dir_name,
                           APR_FILEPATH_NOTABOVEROOT, pool) != APR_SUCCESS) {
        THROW(MESSAGE_UPLOAD_ITEM_SUB_DIR_PATH_CREATION_FAILED);
    }

    return sub_dir_path;
}

void UploadItemIO::prepare_subdir(apr_pool_t *pool, const char *dir_path,
                                 apr_size_t item_id)
{
    apr_finfo_t info;
    const char *sub_dir_path;

    sub_dir_path = get_sub_dir_path(pool, dir_path, item_id);

    if (!APR_STATUS_IS_ENOENT(apr_stat(&info, sub_dir_path, APR_FINFO_MIN,
                                       pool))) {
        return;
    }

    if (apr_dir_make(sub_dir_path, APR_OS_DEFAULT, pool) != APR_SUCCESS) {
        THROW(MESSAGE_UPLOAD_ITEM_SUB_DIR_CREATION_FAILED);
    }
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
