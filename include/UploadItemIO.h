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
 * $Id: UploadItemIO.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef UPLOAD_ITEM_IO_H
#define UPLOAD_ITEM_IO_H

#include "Environment.h"

#include "apr_pools.h"

#include "Uncopyable.h"

class UploadItem;

/**
 * @brief アップロードアイテムの I/O を行うクラス．
 */
class UploadItemIO: public Uncopyable
{
public:
    static const char FILE_EXT_SEPARATOR[];

    /**
     * コンストラクタです．
     *
     * @param[in] pool プール
     * @param[in] data_dir_path アイテムを保存するディレクトリ
     * @param[in] file_dir_path ファイルを保存するディレクトリ
     * @param[in] thumb_dir_path サムネイルを保存するディレクトリ
     */
    UploadItemIO(apr_pool_t *pool, const char *data_dir_path,
                 const char *file_dir_path, const char *thumb_dir_path);
    virtual ~UploadItemIO();

    static const char *get_file_path(apr_pool_t *pool,
                                     const char *file_dir_path,
                                     UploadItem *uitem);
    static const char *get_thumb_path(apr_pool_t *pool,
                                      const char *thumb_dir_path,
                                      apr_size_t item_id);

protected:
    const char *get_data_path(apr_pool_t *pool, apr_size_t item_id) const;
    const char *get_file_path(apr_pool_t *pool, UploadItem *uitem) const;
    const char *get_thumb_path(apr_pool_t *pool, apr_size_t item_id) const;
    void prepare_subdir(apr_pool_t *pool, apr_size_t item_id) const;

    static const char *get_path(apr_pool_t *pool, const char *dir_path,
                                apr_size_t item_id,
                                const char *file_name);
    static const char *get_sub_dir_path(apr_pool_t *pool, const char *dir_path,
                                        apr_size_t item_id);
    static void prepare_subdir(apr_pool_t *pool, const char *dir_path,
                               apr_size_t item_id);

    /** プール */
    apr_pool_t *pool_;

private:
    /** アイテムを保存するディレクトリ */
    const char *data_dir_path_;
    /** ファイルを保存するディレクトリ */
    const char *file_dir_path_;
    /** サムネイルを保存するディレクトリ */
    const char *thumb_dir_path_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
