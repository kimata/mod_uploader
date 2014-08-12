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
 * $Id: ReadWriteLocker.h 2865 2008-04-12 16:10:00Z svn $
 *****************************************************************************/

#ifndef READ_WRITE_LOCKER_H
#define READ_WRITE_LOCKER_H

#include "Environment.h"

#ifdef DEBUG_LOCK
#include <iostream>
#include <iomanip>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

#include "AtomicWrapper.h"
#include "apr_thread_proc.h"
#ifdef DEBUG_ReadWriteLocker
#include "apr_thread_mutex.h"
#endif

#include "Uncopyable.h"
#include "Macro.h"

#ifdef DEBUG_ReadWriteLocker
// MEMO: テスト用にタイムアウト時間を短くする
#undef LCK_TIMEOUT_SEC
#define LCK_TIMEOUT_SEC 5
#endif

/**
 * @brief 読み書きロックの設定を集めたクラス．
 */
class ReadWriteLocker: public Uncopyable
{
public:
    static const apr_uint32_t NOT_LOCKED        = 0x0000;

protected:
    static const apr_uint32_t PREFER_WRITE_SHIFT= 23;
    static const apr_uint32_t PREFER_WRITE_MASK = 1 << PREFER_WRITE_SHIFT;
    static const apr_uint32_t WAIT_WRITE_SHIFT  = 22;
    static const apr_uint32_t WAIT_WRITE_MASK   = 1 << WAIT_WRITE_SHIFT;
    static const apr_uint32_t AGE_SHIFT         = 16;
    static const apr_uint32_t AGE_MASK          = 0x3F << AGE_SHIFT;
    static const apr_uint32_t RESERVED_SHIFT    = 15;
    static const apr_uint32_t RESERVED_MASK     = 1 << RESERVED_SHIFT;
    static const apr_uint32_t READ_COUNT_SHIFT  = 0;
    static const apr_uint32_t READ_COUNT_MASK   = 0x7FFF;
    static const apr_uint32_t VALID_MASK        = PREFER_WRITE_MASK |
                                                  WAIT_WRITE_MASK |
                                                  AGE_MASK |
                                                  READ_COUNT_MASK;

    static const apr_uint32_t INVALID_AGE       = 0xFF;
    static const apr_uint32_t WRITE_LOCKED      = READ_COUNT_MASK;
    static const apr_uint32_t PREFER_WRITE      = 1 << PREFER_WRITE_SHIFT;
    static const apr_uint32_t PREFER_NONE       = 0 << PREFER_WRITE_SHIFT;
    static const apr_uint32_t WAIT_WRITE        = 1 << WAIT_WRITE_SHIFT;
    static const apr_uint32_t WAIT_NONE         = 0 << WAIT_WRITE_SHIFT;
    static const apr_uint32_t AGE_UINIT         = 1 << AGE_SHIFT;
    static const apr_uint32_t READ_COUNT_UINIT  = 1 << READ_COUNT_SHIFT;

    static const apr_uint32_t TIMEOUT_CHECK_MASK= 0x1F;
    static const apr_uint32_t TIMEOUT_SEC       = LCK_TIMEOUT_SEC;

    /**
     * コンストラクタです．
     */
    ReadWriteLocker()
     : start_age_(INVALID_AGE)
#ifdef DEBUG_ReadWriteLocker
       ,
       is_process_timeout_(false)
#endif
    {

    };

    // 注意: ReadWriteLocker 型のポインタ経由で使用されることはないので，
    // デストラクタは virtual にしない．

    /**
     * ロック後の初期化を行います．
     *
     * @param[in] lock ロック変数
     */
    void init(apr_atomic_t *lock, apr_uint32_t status
#ifdef DEBUG_ReadWriteLocker
              , apr_thread_mutex_t *mutex
#endif
              )
    {
        lock_ = lock;
        status_ = status;
#ifdef DEBUG_ReadWriteLocker
        mutex_ = mutex;
#endif
    };
    /**
     * タイムアウト処理すべきかどうか判断します．
     *
     * @param[in] status ロックの状態
     */
    bool should_timeout(apr_uint32_t status);
    /**
     * 他のスレッドがタイムアウト処理したかどうか判断します．
     *
     * @param[in] curr_status 現在のロックの状態
     * @param[in] prev_status 以前のロックの状態
     */
    bool is_others_timeout(apr_uint32_t curr_status, apr_uint32_t prev_status)
    {
        return (((curr_status ^ prev_status) & AGE_MASK) != 0);
    };
#ifdef DEBUG_ReadWriteLocker
    bool is_process_timeout()
    {
        return is_process_timeout_;
    };
#endif
    void log_try_count(const char *label,
                       apr_size_t try_count, apr_uint32_t curr_status,
                       apr_uint32_t new_status)
    {
#ifdef DEBUG_ReadWriteLocker
#ifdef DEBUG_LOCK
        apr_thread_mutex_lock(mutex_);
        std::cerr << "try_count[" << label << "] = " << try_count << std::endl;
        std::cerr << "state: " << std::hex << curr_status << " -> ";
        std::cerr << std::hex << new_status << std::endl;
        std::cerr.flush();
        apr_thread_mutex_unlock(mutex_);
#endif
#endif
    };
    /**
     * プロセッサーを明け渡します．
     */
    static void yield()
    {
#if APR_HAS_THREADS
        // APR 0.x だと UNIX 環境の apr_thread_yield の中身は空なので
#if (APR_MAJOR_VERSION == 0) && defined(_POSIX_PRIORITY_SCHEDULING)
        sched_yield();
#else
        apr_thread_yield();
#endif
#endif
    };
    static apr_uint32_t get_age_count(apr_uint32_t status)
    {
        return ((status & AGE_MASK) >> AGE_SHIFT);
    };
    static apr_uint32_t get_read_count(apr_uint32_t status)
    {
        return ((status & READ_COUNT_MASK) >> READ_COUNT_SHIFT);
    };
    static apr_uint32_t get_prefer_write(apr_uint32_t status)
    {
        return (status & PREFER_WRITE_MASK);
    };
    static apr_uint32_t get_wait_write(apr_uint32_t status)
    {
        return (status & WAIT_WRITE_MASK);
    };

    // MEMO: テスト用
    friend void read_lock(void *data);
    friend void write_lock(void *data);
    template<class LockType> friend void lock_die(void *data1, void *data2);
    friend void run_read_or_write(const char *name, apr_size_t count,
                                  void (* func)(void *data),
                                  apr_pool_t *pool, apr_thread_mutex_t *mutex);
    friend void run_read_and_write(apr_pool_t *pool, apr_thread_mutex_t *mutex);
    friend void run_die(apr_pool_t *pool, apr_thread_mutex_t *mutex);

    /** 読み出し中のスレッドの数へのポインタ */
    apr_atomic_t *lock_;
    apr_uint32_t start_age_;
    apr_time_t start_time_;
    apr_uint32_t status_;
#ifdef DEBUG_ReadWriteLocker
    apr_thread_mutex_t *mutex_;
    bool is_process_timeout_;
#endif
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
