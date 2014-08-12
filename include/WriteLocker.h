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
 * $Id: WriteLocker.h 2822 2008-03-15 14:41:22Z svn $
 *****************************************************************************/

#ifndef WRITE_LOCKER_H
#define WRITE_LOCKER_H

#include "Environment.h"

#include "apr_time.h"
#include "apr_global_mutex.h"

#include "ReadWriteLocker.h"


/**
 * @brief 書き込みロックを表すクラス．
 */
class WriteLocker: public ReadWriteLocker
{
public:
    /**
     * コンストラクタです．
     *
     * @param[in] lock ロック変数
     */
    WriteLocker(apr_atomic_t *lock
#ifdef DEBUG_ReadWriteLocker
                , apr_thread_mutex_t *mutex
#endif
                )
      : ReadWriteLocker()
    {
        apr_size_t try_count;
        apr_uint32_t curr_status;
        apr_uint32_t new_status;
        apr_uint32_t prev_status;

        try_count = 1;
        while (1) {
            curr_status = apr_atomic_read(lock);
            if (get_read_count(curr_status) == NOT_LOCKED) {
                new_status = (curr_status | WRITE_LOCKED) & (~WAIT_WRITE);
                prev_status = apr_atomic_cas(lock, new_status, curr_status);
                init(lock, new_status
#ifdef DEBUG_ReadWriteLocker
                     , mutex
#endif
                     );
                if (prev_status == curr_status) {
                    log_try_count("WriteLock",
                                  try_count, curr_status, new_status);
                    return;
                }
            } else {
                if (((try_count++ & TIMEOUT_CHECK_MASK) == 0) &&
                    should_timeout(curr_status)) {
                    // タイムアウトしたら age を増やす
                    new_status = (curr_status | WRITE_LOCKED) & (~WAIT_WRITE);
                    new_status += AGE_UINIT;
                    prev_status = apr_atomic_cas(lock, new_status, curr_status);
                    init(lock, new_status
#ifdef DEBUG_ReadWriteLocker
                     , mutex
#endif
                         );
                    if (prev_status == curr_status) {
                        log_try_count("WriteLock(TIME OUT)",
                                      try_count, curr_status, new_status);
#ifdef DEBUG_ReadWriteLocker
                        is_process_timeout_ = true;
#endif
                        return;
                    }
                }

                if (get_wait_write(curr_status) != WAIT_WRITE) {
                    new_status = curr_status | WAIT_WRITE;
                    (void)apr_atomic_cas(lock, new_status, curr_status);
                }
            }

            yield();
        }
    };
    ~WriteLocker()
    {
        apr_atomic_t *lock;
        apr_size_t try_count;
        apr_uint32_t curr_status;
        apr_uint32_t new_status;

        lock = lock_;
        try_count = 1;
        while (1) {
            curr_status = apr_atomic_read(lock);
            if (UNLIKELY(is_others_timeout(curr_status, status_))) {
                // 他のスレッドにてタイムアウト処理がされた
                log_try_count("WriteUnLock(TIME OUT)",
                              try_count, curr_status, curr_status);
                return;
            }

            new_status = curr_status & AGE_MASK;
            if (apr_atomic_cas(lock, new_status, curr_status) ==
                curr_status) {
                log_try_count("WriteUnLock",
                              try_count, curr_status, new_status);
                return;
            }
            try_count++;
        }
    };
    static bool is_locked(apr_atomic_t *lock) {
        return get_read_count(apr_atomic_read(lock)) == WRITE_LOCKED;
    };
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
