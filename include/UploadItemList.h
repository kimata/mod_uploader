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
 * $Id: UploadItemList.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef UPLOAD_HEADER_LIST_H
#define UPLOAD_HEADER_LIST_H

#include "Environment.h"

#include <cstdlib>

#include "apr_shm.h"
#include "apr_atomic.h"

#include "UploadItem.h"
#include "Uncopyable.h"


/**
 * @brief アップロードアイテムのリストを表すクラス．
 * このクラスでは排他処理を行いません．
 */
class UploadItemList: public Uncopyable
{
public:
    /**
     * アイテムをリストに追加します．
     *
     * @param[in] uitem アイテム
     */
    void add(UploadItem *uitem);
    /**
     * アイテムをリストから削除します．
     *
     * @param[in] item_id アイテム ID
     */
    void remove(apr_size_t item_id);
    /**
     * 指定された ID のアイテムを返します．
     *
     * @param[in] item_id アイテム ID
     * @return アイテムヘッダ
     */
    const UploadItem::header_t *get_by_id(apr_size_t item_id) const
    {
        return get_by_index(get_index_by_id(item_id));
    };
    UploadItem::header_t *get_by_id(apr_size_t item_id)
    {
        return get_by_index(get_index_by_id(item_id));
    };
    /**
     * 指定されたインデックスのアイテムを返します．
     *
     * @param[in] i インデックス
     * @return アイテムヘッダ
     */
    const UploadItem::header_t *get_by_index(apr_size_t i) const
    {
        return get_header_list() + i;
    };
    UploadItem::header_t *get_by_index(apr_size_t i)
    {
        return get_header_list() + i;
    };
    apr_size_t size() const
    {
        return header_list_size_;
    };
    apr_uint64_t get_total_file_size() const
    {
        return total_file_size_;
    };
    bool is_full(apr_uint64_t file_size) const
    {
        return ((total_file_size_ + file_size) > max_file_size_) ||
            (size() >= max_list_size_);
    };
    /**
     * プールを使ってインスタンスを生成します．
     *
     * @param[in] shm 共有メモリ
     * @param[in] max_file_size リストの最大合計ファイルサイズ
     * @param[in] max_list_size リストの最大サイズ
     * @return UploadItemList のインスタンス
     */
    static UploadItemList *get_instance(apr_shm_t *shm,
                                        apr_uint64_t max_file_size,
                                        apr_size_t max_list_size);
    /**
     * 共有メモリを使って子プロセス用の初期化を行います．
     *
     * @param[in] shm 共有メモリ
     * @return UploadItemList のインスタンス
     */
    static UploadItemList *child_init(apr_shm_t *shm);
    /**
     * リストののデータの最大サイズを計算します．
     *
     * @param[in] max_list_size リストの最大サイズ
     * @return リストのデータの最大サイズ
     */
    static apr_size_t get_memory_size(apr_size_t max_list_size);
    static void dump_header_list(apr_pool_t *pool,
                                 UploadItemList *list);

private:
    UploadItemList(apr_shm_t *shm, apr_uint64_t max_file_size,
                   apr_size_t max_list_size);
    const UploadItem::header_t *get_header_list() const
    {
        return reinterpret_cast<const UploadItem::header_t *>(this + 1);
    };
    UploadItem::header_t *get_header_list()
    {
        return reinterpret_cast<UploadItem::header_t *>(this + 1);
    };
    apr_size_t get_insert_index(UploadItem *uitem) const;
    apr_size_t get_index_by_id(apr_size_t item_id) const;

    static void load_header(UploadItem *uitem, UploadItem::header_t *header);

    static void dump_header(apr_pool_t *pool,
                            const UploadItem::header_t *header);
    static void dump_date(apr_pool_t *pool, apr_time_t time);

    apr_size_t header_list_size_;
    apr_uint64_t total_file_size_;
    apr_uint64_t max_file_size_;
    apr_size_t max_list_size_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
