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
 * $Id: Locker.h 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#ifndef LOCKER_H
#define LOCKER_H

#include "Environment.h"

#include "apr_global_mutex.h"

#include "Macro.h"
#include "Uncopyable.h"

/**
 * @brief 読み書きロックの設定を集めたクラス．
 */
class Locker: public Uncopyable
{
public:
    /**
     * コンストラクタです．
     *
     * @param[in] mutex ミューテックス
     * @param[out] is_locked ロックされているかどうか
     */
    Locker(apr_global_mutex_t *mutex, bool *is_locked=NULL)
      : mutex_(mutex),
        is_locked_(is_locked)
    {
        if (apr_global_mutex_lock(mutex_) != APR_SUCCESS) {
            THROW(MESSAGE_MUTEX_LOCK_FAILED);
        }

        if (is_locked_ != NULL) {
            *is_locked_ = true;
        }
        MEMORY_BARRIER;
    };

    ~Locker()
    {
        if (is_locked_ != NULL) {
            *is_locked_ = false;
        }

        if (apr_global_mutex_unlock(mutex_) != APR_SUCCESS) {
            THROW(MESSAGE_MUTEX_UNLOCK_FAILED);
        }
    };

    apr_global_mutex_t *mutex_;
    bool *is_locked_;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
