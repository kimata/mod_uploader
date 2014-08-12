/******************************************************************************
 * Copyright (C) 2007 Tetsuya Kimata <kimata@acapulco.dyndns.org>
 *
 * All rights reserved.
 *
 * This software is provided 'as-is', without any express or without_lockied
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
 * $Id: UploadItemManager.cpp 2907 2009-01-14 12:56:44Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <memory>

#include "apr_time.h"
#include "apr_file_info.h"

#include "UploadItemManager.h"
#include "UploadItem.h"
#include "UploadItemList.h"
#include "UploadItemIterator.h"
#include "ThumbnailList.h"
#include "TemporaryPool.h"
#include "ReadLocker.h"
#include "WriteLocker.h"
#include "Message.h"
#include "Macro.h"
#include "SourceInfo.h"

using namespace std;

SOURCE_INFO_ADD("$Id: UploadItemManager.cpp 2907 2009-01-14 12:56:44Z svn $");

#ifdef DEBUG
#include <iostream>
#endif

#define AS_UITEM_MANAGER(pointer)   reinterpret_cast<UploadItemManager *>(pointer)

#ifdef DEBUG
#define REQUIRE_MANAGER_LOCKED()                                \
    do {                                                        \
        if (!ReadLocker::is_locked(lock_)) {       \
            throw MESSAGE_MANAGER_SHOULD_LOCKED                 \
                " (" __FILE__ ":" APR_STRINGIFY(__LINE__) ")";  \
        }                                                       \
    } while (0)
#else
#define REQUIRE_MANAGER_LOCKED()
#endif

const apr_size_t UploadItemManager::SYNC_FACTOR = 32;
const apr_size_t UploadItemManager::SYNC_LIMIT  = 200;


/******************************************************************************
 * public メソッド
 *****************************************************************************/
apr_size_t UploadItemManager::add(apr_pool_t *pool,
                                  const char *file_name, const char *file_mime,
                                  apr_uint64_t file_size,
                                  const char *file_digest,
                                  const char *file_temp_path,
                                  const char *remove_pass,
                                  const char *download_pass,
                                  const char *comment, const char *code_pat,
                                  const char *ip_address)
{
    apr_size_t item_id;

    item_id = add_impl(pool, file_name, file_mime, file_size, file_digest,
                       file_temp_path, remove_pass, download_pass, comment,
                       code_pat, ip_address);

    sync_all_item_by_catches(pool);

    return item_id;
}

void UploadItemManager::remove(apr_pool_t *pool, UploadItem *uitem)
{
    remove_impl(pool, uitem);

    sync_all_item_by_catches(pool);
}

void UploadItemManager::sync_all_item(apr_pool_t *pool, apr_size_t sync_limit)
{
    UploadItem::header_t *header;
    apr_size_t synced_count;
    ReadLocker manager_locker(lock_);

    REQUIRE_MANAGER_LOCKED();

    synced_count = 0;
    for (apr_size_t i = 0; i < item_list_->size(); i++) {
        header = item_list_->get_by_index(i);

        if (header->is_need_sync) {
            item_writer_.write_data(pool, header);

            if (++synced_count == sync_limit) {
                break;
            }
        }
    }
}

UploadItem *UploadItemManager::get_item(apr_pool_t *pool, apr_size_t item_id)
{
    UploadItem *uitem;

    ReadLocker manager_locker(lock_);
    REQUIRE_MANAGER_LOCKED();

    APR_PALLOC(uitem, UploadItem *, pool, sizeof(UploadItem));
    memcpy(uitem, item_list_->get_by_id(item_id), sizeof(UploadItem));

    return uitem;
}

apr_file_t *UploadItemManager::get_item_file(apr_pool_t *pool,
                                             apr_size_t item_id,
                                             bool is_sendfile_enabled)
{
    const char *file_path;
    apr_file_t *file;
    UploadItem *uitem;
    // カウンタを保護するだけなので読み出しロックで OK
    ReadLocker manager_locker(lock_);

    REQUIRE_MANAGER_LOCKED();

    uitem = AS_UITEM(item_list_->get_by_id(item_id));

    file_path = UploadItemIO::get_file_path(pool, file_dir_path_, uitem);

    uitem->increment_download_count();

    apr_file_open(&file, file_path,
                  APR_READ | (is_sendfile_enabled ? APR_SENDFILE_ENABLED : 0),
                  APR_OS_DEFAULT, pool);

    return file;
}

void UploadItemManager::update_atime(apr_size_t item_id)
{
    WriteLocker manager_locker(lock_);
    REQUIRE_MANAGER_LOCKED();

    AS_UITEM(item_list_->get_by_id(item_id))->update_atime();
}

UploadItemManager *UploadItemManager::get_instance(apr_shm_t *shm,
                                                   apr_pool_t *pool,
                                                   const char *data_dir_path,
                                                   const char *file_dir_path,
                                                   const char *thumb_dir_path,
                                                   UploadItemList *item_list,
                                                   ThumbnailList *thumbnail_list,
                                                   apr_time_t mtime)
{
    UploadItemManager *item_manager;

#ifdef DEBUG
    if (apr_shm_size_get(shm) != get_memory_size()) {
        THROW(MESSAGE_SHM_SIZE_INVALID);
    }
#endif

    APR_PALLOC(item_manager, UploadItemManager *, pool,
               sizeof(UploadItemManager));

    new(item_manager) UploadItemManager(shm, pool, data_dir_path,
                                        file_dir_path, thumb_dir_path,
                                        item_list, thumbnail_list, mtime);

    return item_manager;
}

void UploadItemManager::child_init(apr_shm_t *shm, UploadItemList *item_list,
                                   ThumbnailList *thumbnail_list)
{
    lock_ = AS_ATOMIC(apr_shm_baseaddr_get(shm));
    mtime_ = AS_TIME(lock_ + 1);
    age_ = AS_SIZE(mtime_ + 1);

#ifdef __INTEL_COMPILER
#pragma warning(disable:1684)
#endif
    ASSERT_EQUAL(AS_CHAR(age_ + 1),
                 AS_CHAR(apr_shm_baseaddr_get(shm)) + get_memory_size());
#ifdef __INTEL_COMPILER
#pragma warning(default:1684)
#endif

    item_list_ = item_list;
    thumbnail_list_ = thumbnail_list;
}

apr_size_t UploadItemManager::get_memory_size()
{
    return sizeof(apr_time_t) + sizeof(apr_atomic_t) + sizeof(apr_size_t);
}

void UploadItemManager::check_finalize_state(UploadItemManager *item_manager)
{
    if (ReadLocker::is_locked(item_manager->lock_)) {
        THROW(MESSAGE_MANAGER_SHOULD_NOT_LOCKED);
    }
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
UploadItemManager::UploadItemManager(apr_shm_t *shm, apr_pool_t *pool,
                                     const char *data_dir_path,
                                     const char *file_dir_path,
                                     const char *thumb_dir_path,
                                     UploadItemList *item_list,
                                     ThumbnailList *thumbnail_list,
                                     apr_time_t mtime)
  : pool_(pool),
    data_dir_path_(data_dir_path),
    file_dir_path_(file_dir_path),
    thumb_dir_path_(thumb_dir_path),
    item_writer_(pool, data_dir_path, file_dir_path, thumb_dir_path)
{
    child_init(shm, item_list, thumbnail_list);

    *lock_ = 0;
    *mtime_ = mtime;
    *age_ = 0;
}

apr_size_t UploadItemManager::add_impl(apr_pool_t *pool,
                                       const char *file_name,
                                       const char *file_mime,
                                       apr_uint64_t file_size,
                                       const char *file_digest,
                                       const char *file_temp_path,
                                       const char *remove_pass,
                                       const char *download_pass,
                                       const char *comment,
                                       const char *code_pat,
                                       const char *ip_address)
{
    apr_size_t item_id;
    apr_size_t item_index;
    UploadItem *uitem;
    apr_time_t current_time;

    remove_files(pool, file_size);

    {
        WriteLocker manager_locker(lock_);

        REQUIRE_MANAGER_LOCKED();

        create_item_id_index(&item_id, &item_index);
        current_time = apr_time_now();

        uitem = UploadItem::get_instance
            (pool, item_id, item_index, file_size, current_time, ip_address,
             file_name, file_mime, file_digest, remove_pass, download_pass,
             comment, code_pat);

        if (item_writer_.write(uitem, file_temp_path)) {
            thumbnail_list_->add(uitem->get_id());
        }
        item_list_->add(uitem);

        set_mtime(current_time);
    }

    // 2 回に分けてロックしているので，最悪の場合を考えると必要
    remove_files(pool, 0);

    return item_id;
}

void UploadItemManager::remove_impl(apr_pool_t *pool, UploadItem *uitem)
{
    WriteLocker manager_locker(lock_);

    REQUIRE_MANAGER_LOCKED();

    // 処理の順番を変えるとまずい
    item_writer_.remove(pool, uitem);
    item_list_->remove(uitem->get_id());
    thumbnail_list_->remove(uitem->get_id());

    set_mtime(apr_time_now());
}

void UploadItemManager::remove_files(apr_pool_t *pool,
                                     apr_uint64_t file_size)
{
#ifdef REMOVE_UNPOPULAR
    remove_unpopular_files(pool, file_size);
#else
    remove_old_files(pool, file_size);
#endif
}

void UploadItemManager::remove_old_files(apr_pool_t *pool,
                                         apr_uint64_t file_size)
{
    WriteLocker manager_locker(lock_);

    REQUIRE_MANAGER_LOCKED();

    // 古いアイテムを削除
    while (item_list_->is_full(file_size)) {
        UploadItem *old_uitem;

        old_uitem = AS_UITEM(item_list_->get_by_index(item_list_->size()-1));

        item_writer_.remove(pool, old_uitem);
        thumbnail_list_->remove(old_uitem->get_id());
        item_list_->remove(old_uitem->get_id());
    }
}

void UploadItemManager::remove_unpopular_files(apr_pool_t *pool,
                                               apr_uint64_t file_size)
{
    // アクセスが無いファイルを削除
    while (item_list_->is_full(file_size)) {
        UploadItem *old_uitem;

        UploadItemIterator item_iter(pool, this, 0, item_list_->size(),
                                     UploadItem::cmp_file_atime);

        WriteLocker manager_locker(lock_);
        REQUIRE_MANAGER_LOCKED();

        old_uitem = item_iter.get();

        item_writer_.remove(pool, old_uitem);
        thumbnail_list_->remove(old_uitem->get_id());
        item_list_->remove(old_uitem->get_id());
    }
}

void UploadItemManager::create_item_id_index(apr_size_t *item_id,
                                             apr_size_t *item_index) const
{
    UploadItem::header_t *header;

    *item_id = static_cast<apr_size_t>(apr_time_sec(apr_time_now()));

    if (item_list_->size() == 0) {
        *item_index = 1;
        return;
    }

    header = item_list_->get_by_index(0);
    while (*item_id <= header->id) {
        (*item_id)++;
    }
    *item_index = header->index + 1;
}

void UploadItemManager::sync_all_item_by_catches(apr_pool_t *pool)
{
    if ((*age_ % SYNC_FACTOR) == 0) {
        sync_all_item(pool, SYNC_LIMIT);
    }

    *age_ += 1;
}

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
