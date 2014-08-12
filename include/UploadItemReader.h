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
 * $Id: UploadItemReader.h 2813 2008-03-07 14:35:04Z svn $
 *****************************************************************************/

#ifndef UPLOAD_ITEM_READER_H
#define UPLOAD_ITEM_READER_H

#include "Environment.h"

#include <cstdlib>

#include "apr_mmap.h"

#include "UploadItemIO.h"
#include "File.h"


class UploadItem;
class File;

/**
 * @brief アップロードアイテムをファイルから読み出すクラス．
 */
class UploadItemReader: public UploadItemIO
{
public:
    /**
     * コンストラクタです．
     *
     * @param[in] pool プール
     * @param[in] data_dir_path アイテムを保存するディレクトリ
     * @param[in] file_dir_path ファイルを保存するディレクトリ
     * @param[in] thumb_dir_path サムネイルを保存するディレクトリ
     */
    UploadItemReader(apr_pool_t *pool, const char *data_dir_path,
                     const char *file_dir_path=NULL,
                     const char *thumb_dir_path=NULL);
    /**
     * アイテムを読み出します．
     *
     * @param[in] item_id アイテムの ID
     * @param[out] uitem アイテムデータ
     */
    void read(apr_size_t item_id, UploadItem *uitem);
    /**
     * アイテムが存在するかどうか調べます．
     *
     * @param[in] item_id アイテムの ID
     * @retval true アイテムが存在する
     * @retval false アイテムが存在しない
     */
    bool is_exist_item(apr_size_t item_id) const;
    /**
     * サムネイルが存在するかどうか調べます．
     *
     * @param[in] item_id アイテムの ID
     * @retval true サムネイルが存在する
     * @retval false サムネイルが存在しない
     */
    bool is_exist_thumbnail(apr_size_t item_id) const;

    static UploadItem *read(apr_pool_t *pool, const char *dir_path,
                           apr_size_t item_id);

private:
    static apr_size_t read_and_check(File *uitem_file,
                                     apr_mmap_t **uitem_file_map);

    static apr_size_t check_format(apr_mmap_t *uitem_file_map);
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
