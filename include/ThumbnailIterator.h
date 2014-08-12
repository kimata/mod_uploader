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
 * $Id: ThumbnailIterator.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef THUMBNAIL_ITERATOR_H
#define THUMBNAIL_ITERATOR_H

#include "Environment.h"

#include <cstdlib>

#include "apr_time.h"

#include "Uncopyable.h"

class UploadItemManager;
class ThumbnailList;

/**
 * @brief サムネイルの読み出しを行うクラス．
 */
class ThumbnailIterator: public Uncopyable
{
public:
    /**
     * コンストラクタです．
     *
     * @param[in] pool プール
     * @param[in] item_manager アイテムマネージャ
     * @param[in] start_index 開始インデックス
     * @param[in] end_index 終了インデックス
     */
    ThumbnailIterator(apr_pool_t *pool, UploadItemManager *item_manager,
                      apr_size_t start_index, apr_size_t end_index);
    /**
     * 現在位置のアイテム ID を取得します．
     *
     * @return アイテム ID
     */
    apr_size_t get() const;
    /**
     * 現在位置を次のアイテム ID に移動させます．
     *
     * @retval true 成功した場合
     * @retval false 失敗した場合（現在位置が最後のアイテム ID）
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
    void validate_index(ThumbnailList *thumbnail_list,
                        apr_size_t *start_index, apr_size_t *end_index);

    apr_size_t *thumbnail_list_;

    apr_size_t curr_index_;
    apr_size_t size_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
