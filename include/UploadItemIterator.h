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
 * $Id: UploadItemIterator.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef UPLOAD_ITEM_ITERATOR_H
#define UPLOAD_ITEM_ITERATOR_H

#include "Environment.h"

#include <cstdlib>

#include "apr_time.h"

#include "UploadItem.h"
#include "Uncopyable.h"


class UploadItemManager;
class UploadItemList;

/**
 * @brief アイテムの読み出しを行うクラス．
 */
class UploadItemIterator: public Uncopyable
{
public:
    /**
     * コンストラクタです．
     *
     * @param[in] pool プール
     * @param[in] item_manager アイテムマネージャ
     * @param[in] start_index 開始インデックス
     * @param[in] end_index 終了インデックス
     * @param[in] cmp ソートに利用する比較関数
     */
    UploadItemIterator(apr_pool_t *pool, UploadItemManager *item_manager,
                       apr_size_t start_index, apr_size_t end_index,
                       int (*cmp)(const UploadItem *a, const UploadItem *b));
    /**
     * コンストラクタです．
     *
     * @param[in] pool プール
     * @param[in] item_manager アイテムマネージャ
     * @param[in] start_index 開始インデックス
     * @param[in] end_index 終了インデックス
     */
    UploadItemIterator(apr_pool_t *pool, UploadItemManager *item_manager,
                       apr_size_t start_index, apr_size_t end_index);
    /**
     * コンストラクタです．
     *
     * @param[in] pool プール
     * @param[in] item_list アイテムリスト
     * @param[in] start_index 開始インデックス
     * @param[in] end_index 終了インデックス
     */
    UploadItemIterator(apr_pool_t *pool, UploadItemList *item_list,
                       apr_size_t start_index, apr_size_t end_index);
    /**
     * 現在位置のアイテムを取得します．
     *
     * @return アイテム
     */
    UploadItem *get() const;
    /**
     * 現在位置を次のアイテムに移動させます．
     *
     * @retval true 成功した場合
     * @retval false 失敗した場合（現在位置が最後のアイテム）
     */
    bool next();
    /**
     * 読み出し位置をリセットします．
     */
    void reset();

    apr_size_t size() const
    {
        return size_;
    };

private:
    void init(apr_pool_t *pool, UploadItemList *item_list,
              apr_size_t start_index, apr_size_t end_index,
              int (*cmp)(const UploadItem *a, const UploadItem *b));
    void init(apr_pool_t *pool, UploadItemList *item_list,
              apr_size_t start_index, apr_size_t end_index);
    void validate_index(UploadItemList *item_list,
                        apr_size_t *start_index, apr_size_t *end_index);

    UploadItem::header_t *header_list_;

    apr_size_t curr_index_;
    apr_size_t size_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
