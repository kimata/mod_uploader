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
 * $Id: ThumbnailList.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <cstddef>
#include <iostream>
#include <iomanip>

#include "ThumbnailList.h"
#include "UploadItemList.h"
#include "Message.h"
#include "Auxiliary.h"
#include "SourceInfo.h"

using namespace std;

SOURCE_INFO_ADD("$Id: ThumbnailList.cpp 2756 2007-12-11 10:57:59Z svn $");

#define AS_THUMBNAIL_LIST(pointer)  reinterpret_cast<ThumbnailList *>(pointer)

/******************************************************************************
 * public メソッド
 *****************************************************************************/
ThumbnailList::ThumbnailList(apr_shm_t *shm, apr_size_t max_list_size)
  : thumbnail_list_size_(0),
    max_list_size_(max_list_size)
{
#ifdef DEBUG
    if (apr_shm_size_get(shm) != get_memory_size(max_list_size)) {
        THROW(MESSAGE_SHM_SIZE_INVALID);
    }
#endif
}

void ThumbnailList::add(apr_size_t item_id)
{
    apr_size_t i;
    apr_size_t *thumbnail_list;
    apr_size_t thumbnail_list_size;

    thumbnail_list = get_thumbnail_list();
    thumbnail_list_size = size();

    if (thumbnail_list_size == max_list_size_) {
        THROW(MESSAGE_BUG_FOUND);
    }

    // MEMO: thumbnail_list は item_id に基づいて降順に並んでいる

    i = get_insert_index(item_id);
    if (i == thumbnail_list_size) {
        if (thumbnail_list_size == max_list_size_) { // 追加する必要なし
            return;
        } else { // 末尾に追加
            thumbnail_list[i] = item_id;
        }
    } else { // 挿入
        memmove(thumbnail_list + i + 1, thumbnail_list + i,
                sizeof(apr_size_t) * (thumbnail_list_size - i));
        thumbnail_list[i] = item_id;
    }

    thumbnail_list_size_++;
}

void ThumbnailList::remove(apr_size_t item_id)
{
    apr_size_t i;
    apr_size_t *thumbnail_list;
    apr_size_t thumbnail_list_size;

    thumbnail_list = get_thumbnail_list();
    thumbnail_list_size = size();

    if (thumbnail_list_size == 0) {
        return;
    }

    i = get_index_by_id(item_id);
    if (i == thumbnail_list_size) {
        // リスト中に存在しない
        return;
    }

    if (i != (thumbnail_list_size - 1)) {
        memmove(thumbnail_list + i, thumbnail_list + i + 1,
                sizeof(apr_size_t) * (thumbnail_list_size - i - 1));
    }

    if (thumbnail_list_size == 0) { // 念のため
        THROW(MESSAGE_BUG_FOUND);
    }

    thumbnail_list_size_--;
}

ThumbnailList *ThumbnailList::get_instance(apr_shm_t *shm,
                                           apr_size_t max_list_size)
{
    ThumbnailList *thumbnail_list;

    thumbnail_list = AS_THUMBNAIL_LIST(apr_shm_baseaddr_get(shm));
    new(thumbnail_list) ThumbnailList(shm, max_list_size);

    return thumbnail_list;
}

ThumbnailList *ThumbnailList::child_init(apr_shm_t *shm)
{
    return AS_THUMBNAIL_LIST(apr_shm_baseaddr_get(shm));
}

apr_size_t ThumbnailList::get_memory_size(apr_size_t max_list_size)
{
    return sizeof(ThumbnailList) + (sizeof(apr_size_t)*max_list_size);
}

#include <fstream>

void ThumbnailList::dump_thumbnail_list(apr_pool_t *pool,
                                        ThumbnailList *list)
{
    const apr_size_t *thumbnail_list;
    apr_size_t thumbnail_list_size;

    thumbnail_list = list->get_thumbnail_list();
    thumbnail_list_size = list->size();

    for (apr_size_t i = 0; i < thumbnail_list_size; i++) {
        cerr << "[" << setw(3) << i << "] ";
        cerr << thumbnail_list[i] << endl;
    }
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
apr_size_t ThumbnailList::get_insert_index(apr_size_t item_id) const
{
    apr_size_t i;
    const apr_size_t *thumbnail_list;
    apr_size_t thumbnail_list_size;

    thumbnail_list = get_thumbnail_list();
    thumbnail_list_size = size();

    for (i = 0; i < thumbnail_list_size; i++) {
        if (item_id >= thumbnail_list[i]) {
            return i;
        }
    }
    return i;
}

apr_size_t ThumbnailList::get_index_by_id(apr_size_t item_id) const
{
    const apr_size_t *thumbnail_list;
    apr_size_t thumbnail_list_size;

    thumbnail_list = get_thumbnail_list();
    thumbnail_list_size = size();

    // 頻繁には呼ばれ無いはずなので，線形探索...
    for (apr_size_t i = 0; i < thumbnail_list_size; i++) {
        if (thumbnail_list[i] == item_id) {
            return i;
        }
    }
    return thumbnail_list_size;
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
