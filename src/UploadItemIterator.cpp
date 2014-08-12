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
 * $Id: UploadItemIterator.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <algorithm>

#include "apr.h"

#include "UploadItemIterator.h"
#include "UploadItemManager.h"
#include "UploadItemList.h"
#include "ReadLocker.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: UploadItemIterator.cpp 2756 2007-12-11 10:57:59Z svn $");


/******************************************************************************
 * public メソッド
 *****************************************************************************/
UploadItemIterator::UploadItemIterator(apr_pool_t *pool,
                                       UploadItemManager *item_manager,
                                       apr_size_t start_index,
                                       apr_size_t end_index,
                                       int (*cmp)(const UploadItem *a, const UploadItem *b))
  : header_list_(NULL),
    curr_index_(0)
{
    ReadLocker manager_locker(item_manager->lock_);

    init(pool, item_manager->item_list_, start_index, end_index, cmp);
}

UploadItemIterator::UploadItemIterator(apr_pool_t *pool,
                                       UploadItemManager *item_manager,
                                       apr_size_t start_index,
                                       apr_size_t end_index)
  : header_list_(NULL),
    curr_index_(0)
{
    ReadLocker manager_locker(item_manager->lock_);

    init(pool, item_manager->item_list_, start_index, end_index);
}

UploadItemIterator::UploadItemIterator(apr_pool_t *pool,
                                       UploadItemList *item_list,
                                       apr_size_t start_index,
                                       apr_size_t end_index)
  : header_list_(NULL),
    curr_index_(0)
{
    init(pool, item_list, start_index, end_index);
}

UploadItem *UploadItemIterator::get() const
{
    return AS_UITEM(header_list_ + curr_index_);
}

bool UploadItemIterator::next()
{
    return (++curr_index_ < size_);
}

void UploadItemIterator::reset()
{
    curr_index_ = 0;
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void UploadItemIterator::init(apr_pool_t *pool, UploadItemList *item_list,
                              apr_size_t start_index, apr_size_t end_index,
                              int (*cmp)(const UploadItem *a, const UploadItem *b))
{
    apr_size_t list_size;

    validate_index(item_list, &start_index, &end_index);
    size_= end_index - start_index;

    if (size_ == 0) {
        return;
    }

    // MEMO: 無駄が多いけど，ローカルに全部コピーしてソート

    list_size = item_list->size();
    APR_PALLOC(header_list_, UploadItem::header_t *, pool,
               sizeof(UploadItem::header_t) * list_size);
    memcpy(header_list_, item_list->get_by_index(0),
           sizeof(UploadItem::header_t) * list_size);

    qsort(header_list_, list_size, sizeof(UploadItem),
          reinterpret_cast<int (*)(const void *a, const void *b)>(cmp));

    header_list_ += start_index;
}

void UploadItemIterator::init(apr_pool_t *pool, UploadItemList *item_list,
                              apr_size_t start_index, apr_size_t end_index)
{
    validate_index(item_list, &start_index, &end_index);
    size_= end_index - start_index;

    if (size_ == 0) {
        return;
    }

    APR_PALLOC(header_list_, UploadItem::header_t *, pool,
               sizeof(UploadItem::header_t)*size_);
    memcpy(header_list_, item_list->get_by_index(start_index),
           sizeof(UploadItem::header_t) * size_);
}

void UploadItemIterator::validate_index(UploadItemList *item_list,
                                        apr_size_t *start_index,
                                        apr_size_t *end_index)
{
#ifdef DEBUG
    if (*start_index >= *end_index) {
        THROW(MESSAGE_BUG_FOUND);
    }
#endif

    if (*end_index > item_list->size()) {
        *end_index = item_list->size();
    }
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
