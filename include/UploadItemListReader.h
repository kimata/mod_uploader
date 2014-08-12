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
 * $Id: UploadItemListReader.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef UPOLOAD_ITEM_LIST_READER_H
#define UPOLOAD_ITEM_LIST_READER_H

#include "Environment.h"

#include <cstdlib>

#include "apr_pools.h"
#include "apr_shm.h"

#include "Uncopyable.h"


class UploadItemList;
class UploadItemReader;
class ThumbnailList;

/**
 * @brief アップロードアイテムのリストを読み出すクラス．
 */
class UploadItemListReader: public Uncopyable
{
public:
    /**
     * アップロードアイテムを読み出します．
     *
     * @param[in] pool プール
     * @param[in] data_dir_path アイテムが保存されているディレクトリ
     * @param[in] thumb_dir_path サムネイルが保存されているディレクトリ
     * @param[in] max_file_size リストの最大合計ファイルサイズ
     * @param[in] max_list_size リストの最大サイズ
     * @param[in] shm 共有メモリ
     * @param[in] thumbnail_list サムネイルのリスト
     * @param[in] mtime 最終修正日時
     * @return 掲示板のアップロードアイテム
     */
    static UploadItemList *read(apr_pool_t *pool, const char *data_dir_path,
                                const char *thumb_dir_path,
                                apr_uint64_t max_file_size,
                                apr_size_t max_list_size,
                                apr_shm_t *shm, ThumbnailList *thumbnail_list,
                                apr_time_t *mtime);

private:
    UploadItemListReader();

    static void get_subdir_threads(apr_pool_t *pool, const char *sub_dir_path,
                                   UploadItemReader *uitem_reader,
                                   UploadItemList *uitem_list,
                                   ThumbnailList *thumbnail_list);
    static bool is_subdir_name_valid(const char *subdir_name);
    static bool is_file_name_valid(const char *file_name);
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
