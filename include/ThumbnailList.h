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
 * $Id: ThumbnailList.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef THUMBNAIL_LIST_H
#define THUMBNAIL_LIST_H

#include "Environment.h"

#include <cstdlib>

#include "apr_shm.h"
#include "apr_atomic.h"

#include "Uncopyable.h"


class UploadItemList;

/**
 * @brief サムネイルのリストを表すクラス．
 * このクラスでは排他処理を行いません．
 *
 * メモリ上のデータの管理のみを行います．ファイルシステムの操作は行いません．
 */
class ThumbnailList: public Uncopyable
{
public:
    ThumbnailList(apr_shm_t *shm, apr_size_t max_list_size);
    /**
     * サムネイルをリストに追加します．
     *
     * @param[in] item_id アイテム ID
     */
    void add(apr_size_t item_id);
    /**
     * サムネイルをリストから削除します．
     *
     * @param[in] item_id アイテムy ID
     */
    void remove(apr_size_t item_id);
    /**
     * 指定されたインデックスのサムネイルを返します．
     *
     * @param[in] i インデックス
     * @return アイテム ID
     */
    const apr_size_t *get_by_index(apr_size_t i) const
    {
        return get_thumbnail_list() + i;
    };
    apr_size_t size() const
    {
        return thumbnail_list_size_;
    };
    bool is_full() const
    {
        return (size() == max_list_size_);
    };
    /**
     * プールを使ってインスタンスを生成します．
     *
     * @param[in] shm 共有メモリ
     * @param[in] max_list_size リストの最大サイズ
     * @return ThumbnailList のインスタンス
     */
    static ThumbnailList *get_instance(apr_shm_t *shm,
                                       apr_size_t max_list_size);
    /**
     * 共有メモリを使って子プロセス用の初期化を行います．
     *
     * @param[in] shm 共有メモリ
     * @return ThumbnailList のインスタンス
     */
    static ThumbnailList *child_init(apr_shm_t *shm);
    /**
     * リストののデータの最大サイズを計算します．
     *
     * @param[in] max_list_size リストの最大サイズ
     * @return リストのデータの最大サイズ
     */
    static apr_size_t get_memory_size(apr_size_t max_list_size);
    static void dump_thumbnail_list(apr_pool_t *pool, ThumbnailList *list);

private:
    const apr_size_t *get_thumbnail_list() const
    {
        return reinterpret_cast<const apr_size_t *>(this + 1);
    };
    apr_size_t *get_thumbnail_list()
    {
        return reinterpret_cast< apr_size_t *>(this + 1);
    };
    apr_size_t get_insert_index(apr_size_t item_id) const;
    apr_size_t get_index_by_id(apr_size_t item_id) const;

    apr_size_t thumbnail_list_size_;
    apr_size_t max_list_size_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
