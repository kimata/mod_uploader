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
 * $Id: ThumbnailIterator.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <algorithm>

#include "apr.h"

#include "ThumbnailIterator.h"
#include "UploadItemManager.h"
#include "ThumbnailList.h"
#include "ReadLocker.h"
#include "Macro.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: ThumbnailIterator.cpp 2756 2007-12-11 10:57:59Z svn $");

/******************************************************************************
 * public メソッド
 *****************************************************************************/
ThumbnailIterator::ThumbnailIterator(apr_pool_t *pool,
                                     UploadItemManager *item_manager,
                                     apr_size_t start_index,
                                     apr_size_t end_index)
  : thumbnail_list_(NULL),
    curr_index_(0)
{
    ThumbnailList *thumbnail_list;
    ReadLocker manager_locker(item_manager->lock_);

    thumbnail_list = item_manager->thumbnail_list_;

    validate_index(thumbnail_list, &start_index, &end_index);
    size_= end_index - start_index;

    if (size_ != 0) {
        APR_PALLOC(thumbnail_list_, apr_size_t *, pool,
                   sizeof(apr_size_t)*(end_index-start_index));
        memcpy(thumbnail_list_, thumbnail_list->get_by_index(start_index),
               sizeof(apr_size_t)*size_);
    }
}

apr_size_t ThumbnailIterator::get() const
{
    return thumbnail_list_[curr_index_];
}

bool ThumbnailIterator::next()
{
    return (++curr_index_ < size_);
}

void ThumbnailIterator::reset()
{
    curr_index_ = 0;
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void ThumbnailIterator::validate_index(ThumbnailList *thumbnail_list,
                                       apr_size_t *start_index,
                                       apr_size_t *end_index)
{
#ifdef DEBUG
    if (*start_index >= *end_index) {
        THROW(MESSAGE_BUG_FOUND);
    }
#endif

    if (*end_index > thumbnail_list->size()) {
        *end_index = thumbnail_list->size();
    }
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
