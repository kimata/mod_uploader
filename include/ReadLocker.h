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
 * $Id: ReadLocker.h 2822 2008-03-15 14:41:22Z svn $
 *****************************************************************************/

#ifndef READ_LOCKER_H
#define READ_LOCKER_H

#include "Environment.h"

#include "ReadWriteLocker.h"


/**
 * @brief 読み出しロックを表すクラス．
 */
class ReadLocker: public ReadWriteLocker
{
public:
    /**
     * コンストラクタです．
     *
     * @param[in] lock ロック変数
     */
    ReadLocker(apr_atomic_t *lock
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
            if (LIKELY(can_lock(curr_status))) {
                new_status = curr_status + READ_COUNT_UINIT;
                prev_status = apr_atomic_cas(lock, new_status, curr_status);
                init(lock, new_status
#ifdef DEBUG_ReadWriteLocker
                     , mutex
#endif
                     );
                if (prev_status == curr_status) {
                    log_try_count("ReadLock",
                                  try_count, curr_status, new_status);
                    return;
                }
            } else if (((try_count++ & TIMEOUT_CHECK_MASK) == 0) &&
                       should_timeout(curr_status)) {
                // タイムアウトしたら age を増やす
                new_status = (curr_status & (~READ_COUNT_MASK)) | READ_COUNT_UINIT;
                new_status += AGE_UINIT;
                prev_status = apr_atomic_cas(lock, new_status, curr_status);
                init(lock, new_status
#ifdef DEBUG_ReadWriteLocker
                     , mutex
#endif
                     );
                if (prev_status == curr_status) {
                    log_try_count("ReadLock(TIME OUT)",
                                  try_count, curr_status, new_status);
#ifdef DEBUG_ReadWriteLocker
                    is_process_timeout_ = true;
#endif
                    return;
                }
            }

            yield();
        }
    };
    ~ReadLocker()
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
                log_try_count("ReadUnLock(TIME OUT)",
                              try_count, curr_status, curr_status);
                return;
            }

            // read_count は変更されている可能性があることに注意！

            new_status = (curr_status - READ_COUNT_UINIT) | PREFER_WRITE;
            if (apr_atomic_cas(lock, new_status, curr_status) ==
                curr_status) {
                log_try_count("ReadUnLock",
                              try_count, curr_status, new_status);
                return;
            }
            try_count++;
        }
    };
    static bool is_locked(apr_atomic_t *lock) {
        return get_read_count(apr_atomic_read(lock)) != NOT_LOCKED;
    };
private:
    bool can_lock(apr_uint32_t status)
    {
        return ((get_read_count(status) != WRITE_LOCKED) &&
                ((status & (PREFER_WRITE_MASK|WAIT_WRITE_MASK)) !=
                 (WAIT_WRITE|PREFER_WRITE)));
    };
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
