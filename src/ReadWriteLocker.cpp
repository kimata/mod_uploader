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
 * $Id: ReadWriteLocker.cpp 2865 2008-04-12 16:10:00Z svn $
 *****************************************************************************/

#include "Environment.h"

#include "ReadWriteLocker.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: ReadWriteLocker.cpp 2865 2008-04-12 16:10:00Z svn $");


/******************************************************************************
 * protected メソッド
 *****************************************************************************/
bool ReadWriteLocker::should_timeout(apr_uint32_t status)
{
    apr_uint32_t curr_age;
    apr_time_t curr_time;

    curr_age = status & AGE_MASK;
    curr_time = apr_time_now();

    if ((start_age_ == INVALID_AGE) ||
        is_others_timeout(curr_age, start_age_)) {
        start_age_ = curr_age;
        start_time_ = curr_time;

        return false;
    }

    if (UNLIKELY(apr_time_sec(curr_time - start_time_) > TIMEOUT_SEC)) {
#ifndef DEBUG_ReadWriteLocker
        // MEMO: テスト時は別の手段でタイムアウト処理を確認しているので
        // メッセージは表示しない
        std::cerr << "[warn] " PACKAGE_NAME;
        std::cerr << ": Lock timeout occurred" << std::endl;
#endif
        return true;
    } else {
        return false;
    }
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_ReadWriteLocker
#include "TestRunner.h"

#include "ReadLocker.h"
#include "WriteLocker.h"

const apr_size_t READ_COUNT             = 100;
const apr_size_t WRITE_COUNT            = 20;
const apr_size_t DIE_COUNT              = 20;
const apr_size_t REPEAT_COUNT           = 100;
const apr_size_t TIME_LIMIT_SEC         = 300;

typedef struct {
    apr_atomic_t lock;
    apr_atomic_t read_count;
    apr_atomic_t timeout_count;
    apr_thread_mutex_t *mutex;
} handle_t;

typedef struct {
    handle_t *handle;
    void (*func)(void *data);
} work_t;

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << endl;
}

void read_lock(void *data)
{
    handle_t *handle;

    handle = reinterpret_cast<handle_t *>(data);

    ReadWriteLocker::yield();

    ReadLocker read_lock(&(handle->lock), handle->mutex);

    apr_atomic_inc(&(handle->read_count));

    if (read_lock.is_process_timeout()) {
        apr_atomic_inc(&(handle->timeout_count));
    }

    if (ReadWriteLocker::get_read_count(handle->lock) ==
        ReadWriteLocker::WRITE_LOCKED) {
        THROW(MESSAGE_BUG_FOUND);
    }

    ReadWriteLocker::yield();

    apr_atomic_dec(&(handle->read_count));
}

void write_lock(void *data)
{
    handle_t *handle;

    handle = reinterpret_cast<handle_t *>(data);

    {
        WriteLocker write_lock(&(handle->lock), handle->mutex);

        ReadWriteLocker::yield();

        if (write_lock.is_process_timeout()) {
            apr_atomic_inc(&(handle->timeout_count));
        }

        if (ReadWriteLocker::get_read_count(handle->lock) !=
            ReadWriteLocker::WRITE_LOCKED) {
            THROW(MESSAGE_BUG_FOUND);
        }

        if (apr_atomic_read(&(handle->read_count)) != 0) {
            THROW(MESSAGE_BUG_FOUND);
        }

        ReadWriteLocker::yield();
    }
}

template<class LockType>
void lock_die(void *data1, void *data2)
{
    handle_t *handle;
    apr_thread_t *thread;

    handle = reinterpret_cast<handle_t *>(data1);
    thread = reinterpret_cast<apr_thread_t *>(data2);

    ReadWriteLocker::yield();

    {
        LockType lock(&(handle->lock), handle->mutex);
        if (lock.is_process_timeout()) {
            apr_atomic_inc(&(handle->timeout_count));
        }

        // ロック中にわざと終了させる
        apr_thread_exit(thread, APR_SUCCESS);
    }
}

static void *APR_THREAD_FUNC worker(apr_thread_t *thread, void *data)
{
    work_t *work;

    work = reinterpret_cast<work_t *>(data);
    try {
        for (apr_size_t i = 0; i < REPEAT_COUNT; i++) {
            (work->func)(work->handle);
        }
    } catch(const char *) {
        apr_thread_exit(thread, APR_EBUSY);
    }

    apr_thread_exit(thread, APR_SUCCESS);

    return NULL;
}

// NOTE: die_worker を friend 関数にするのは避けたかったので，
// lock_die を使用

template<class LockType>
static void *APR_THREAD_FUNC die_worker(apr_thread_t *thread,
                                        void *data)
{
    lock_die<LockType>(data, thread);

    return NULL;
}

void run_read_or_write(const char *name, apr_size_t count,
                       void (* func)(void *data),
                       apr_pool_t *pool, apr_thread_mutex_t *mutex)
{
    handle_t handle;
    work_t work;
    apr_thread_t **workers;
    apr_status_t status;
    bool is_ok;
    volatile double start_time;
    volatile double end_time;

    show_test_name(name);

    is_ok = true;

    handle.lock = 0;
    handle.read_count = 0;
    handle.timeout_count = 0;
    handle.mutex = mutex;

    work.handle = &handle;
    work.func = func;

    start_time = get_time_sec();

    APR_PALLOC(workers, apr_thread_t **, pool, sizeof(apr_thread_t *)*count);
    for (apr_size_t i = 0; i < count; i++) {
        apr_thread_create(workers + i, NULL, worker, &work, pool);
    }
    for (apr_size_t i = 0; i < count; i++) {
        apr_thread_join(&status, workers[i]);
        if (status != APR_SUCCESS) {
            is_ok = false;
        }
    }
    if (!is_ok) {
        THROW(MESSAGE_BUG_FOUND);
    }

    if (ReadWriteLocker::get_read_count(handle.lock) != 0) {
        THROW(MESSAGE_BUG_FOUND);
    }

    end_time = get_time_sec();

    if ((end_time - start_time) > TIME_LIMIT_SEC) {
        // 時間がかかりすぎた場合もダメ
        THROW(MESSAGE_BUG_FOUND);
    }

    show_item(name, (end_time - start_time)*1000, " msec");
}

void run_read(apr_pool_t *pool, apr_thread_mutex_t *mutex)
{
    return run_read_or_write("read", READ_COUNT, read_lock, pool, mutex);
}

void run_write(apr_pool_t *pool, apr_thread_mutex_t *mutex)
{
    return run_read_or_write("write", WRITE_COUNT, write_lock, pool, mutex);
}

void run_read_and_write(apr_pool_t *pool, apr_thread_mutex_t *mutex)
{
    handle_t handle;
    work_t read_work;
    work_t write_work;
    apr_thread_t *read_workers[READ_COUNT];
    apr_thread_t *write_workers[WRITE_COUNT];
    apr_status_t status;
    bool is_read_ok;
    bool is_write_ok;
    volatile double start_time;
    volatile double end_time;

    show_test_name("read/write");

    is_read_ok = true;
    is_write_ok = true;

    handle.lock = 0;
    handle.read_count = 0;
    handle.timeout_count = 0;
    handle.mutex = mutex;

    read_work.handle = &handle;
    read_work.func = read_lock;
    write_work.handle = &handle;
    write_work.func = write_lock;

    start_time = get_time_sec();

    for (apr_size_t i = 0; i < READ_COUNT; i++) {
        apr_thread_create(read_workers + i, NULL, worker, &read_work, pool);
    }
    for (apr_size_t i = 0; i < WRITE_COUNT; i++) {
        apr_thread_create(write_workers + i, NULL, worker,
                          &write_work, pool);
    }

    for (apr_size_t i = 0; i < READ_COUNT; i++) {
        apr_thread_join(&status, read_workers[i]);
        if (status != APR_SUCCESS) {
            is_read_ok = false;
        }
    }
    for (apr_size_t i = 0; i < WRITE_COUNT; i++) {
        apr_thread_join(&status, write_workers[i]);
        if (status != APR_SUCCESS) {
            is_write_ok = false;
        }
    }
    if (!is_read_ok) {
        THROW(MESSAGE_BUG_FOUND);
    } else if (!is_write_ok) {
        THROW(MESSAGE_BUG_FOUND);
    }

    if (ReadWriteLocker::get_read_count(handle.lock) != 0) {
        THROW(MESSAGE_BUG_FOUND);
    }
    if (apr_atomic_read(&(handle.timeout_count)) != 0) {
        THROW(MESSAGE_BUG_FOUND);
    }

    end_time = get_time_sec();

    if ((end_time - start_time) > TIME_LIMIT_SEC) {
        // 時間がかかりすぎた場合もダメ
        THROW(MESSAGE_BUG_FOUND);
    }

    show_item("write", (end_time - start_time)*1000, " msec");
}

void run_die(apr_pool_t *pool, apr_thread_mutex_t *mutex)
{
    handle_t handle;
    work_t read_work;
    work_t write_work;
    apr_thread_t *read_workers[READ_COUNT];
    apr_thread_t *write_workers[WRITE_COUNT];
    apr_thread_t *read_die_workers[DIE_COUNT];
    apr_thread_t *write_die_workers[DIE_COUNT];
    apr_status_t status;
    bool is_read_ok;
    bool is_write_ok;
    bool is_die_ok;
    volatile double start_time;
    volatile double end_time;

    show_test_name("die");

    is_read_ok = true;
    is_write_ok = true;
    is_die_ok = true;

    handle.lock = 0;
    handle.read_count = 0;
    handle.timeout_count = 0;
    handle.mutex = mutex;

    read_work.handle = &handle;
    read_work.func = read_lock;
    write_work.handle = &handle;
    write_work.func = write_lock;

    start_time = get_time_sec();

    for (apr_size_t i = 0; i < READ_COUNT; i++) {
        apr_thread_create(read_workers + i, NULL, worker, &read_work, pool);
    }
    for (apr_size_t i = 0; i < WRITE_COUNT; i++) {
        apr_thread_create(write_workers + i, NULL, worker,
                          &write_work, pool);
    }
    for (apr_size_t i = 0; i < DIE_COUNT; i++) {
        apr_thread_create(read_die_workers + i, NULL, die_worker<ReadLocker>,
                          &handle, pool);
    }
    for (apr_size_t i = 0; i < DIE_COUNT; i++) {
        apr_thread_create(write_die_workers + i, NULL, die_worker<WriteLocker>,
                          &handle, pool);
    }

    for (apr_size_t i = 0; i < READ_COUNT; i++) {
        apr_thread_join(&status, read_workers[i]);
        if (status != APR_SUCCESS) {
            is_read_ok = false;
        }
    }
    for (apr_size_t i = 0; i < WRITE_COUNT; i++) {
        apr_thread_join(&status, write_workers[i]);
        if (status != APR_SUCCESS) {
            is_write_ok = false;
        }
    }
    for (apr_size_t i = 0; i < DIE_COUNT; i++) {
        apr_thread_join(&status, read_die_workers[i]);
        if (status != APR_SUCCESS) {
            is_die_ok = false;
        }
    }
    if (!is_read_ok) {
        THROW(MESSAGE_BUG_FOUND);
    } else if (!is_write_ok) {
        THROW(MESSAGE_BUG_FOUND);
    } else if (!is_die_ok) {
        THROW(MESSAGE_BUG_FOUND);
    }

    if ((ReadWriteLocker::get_read_count(handle.lock) > DIE_COUNT) &&
        (ReadWriteLocker::get_read_count(handle.lock) !=
         ReadWriteLocker::WRITE_LOCKED)) {
        // MEMO: 最後にロックを保持したまま終了した場合，
        // get_read_count の返り値は DIE_COUNT 以下や WRITE_LOCKED になっ
        // たりする可能性がある．
        THROW(MESSAGE_BUG_FOUND);
    }
    if (apr_atomic_read(&(handle.timeout_count)) > (DIE_COUNT * 2)) {
        // MEMO: Read と Write でそれぞれ DIE_COUNT 回ずつロックを保持
        // したまま終了させているが，Read に関してはネストしてロックで
        // きるので，タイムアウト回数の期待値としては DIE_COUNT + 1 以
        // 上，DIE_COUNT * 2 以下となる．
        THROW(MESSAGE_BUG_FOUND);
    }

    end_time = get_time_sec();

    if ((end_time - start_time) > TIME_LIMIT_SEC) {
        // 時間がかかりすぎた場合もダメ
        THROW(MESSAGE_BUG_FOUND);
    }

    show_item("die", (end_time - start_time)*1000, " msec");
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    apr_thread_mutex_t *mutex;

    if (argc != 1) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    show_line();

    apr_thread_mutex_create(&mutex, APR_THREAD_MUTEX_UNNESTED, pool);
    srand(static_cast<unsigned int>(apr_time_msec(apr_time_now())));


#if APR_HAS_THREADS
    run_read(pool, mutex);
    run_write(pool, mutex);
    run_read_and_write(pool, mutex);
    run_die(pool, mutex);
#else
    show_item("read", "skipped");
    show_item("write", "skipped");
    show_item("read/write", "skipped");
    show_item("die", "skipped");
#endif
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
