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
 * $Id: PostProgressList.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <iomanip>

#include "PostProgressList.h"
#include "ReadLocker.h"
#include "WriteLocker.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: PostProgressList.cpp 2756 2007-12-11 10:57:59Z svn $");

using namespace std;

const apr_uint32_t PostProgressList::ID_DUMMY   = 0;
const apr_time_t PostProgressList::ENTRY_UNUSED = 0;
const apr_time_t PostProgressList::ENTRY_USED   = 1;

/******************************************************************************
 * public メソッド
 *****************************************************************************/
PostProgressList::PostProgressList()
{
    memset(progress_list_, 0, sizeof(progress_list_));
    (void)apr_atomic_set(&lock_, ReadWriteLocker::NOT_LOCKED);
}

RequestReader::post_progress_t *PostProgressList::get(apr_uint32_t upload_id)
{
    ReadLocker lock(&lock_);

    if (upload_id == ID_DUMMY) {
        return progress_list_;
    }

    for (apr_size_t i = 1; i < ARRAY_SIZE_OF(progress_list_); i++) {
        if (progress_list_[i].id == upload_id) {
            return (progress_list_ + i);
        }
    }

    // エントリが見つからなければダミーを返す
    return progress_list_;
}

RequestReader::post_progress_t *PostProgressList::add(apr_uint32_t upload_id)
{
    apr_time_t current_time;

    WriteLocker lock(&lock_);

    if (upload_id == ID_DUMMY) {
        return progress_list_;
    }

    current_time = apr_time_now();

    // エントリの状態
    // progress_list[i].end_time == ENTRY_UNUSED: 未使用
    // progress_list[i].end_time == ENTRY_USED  : 使用中(アップロード中)
    // progress_list[i].end_time == それ以外    : 使用中(アップロード完了)

    // 既に同じ upload_id があるかチェックしつつ古いエントリを未使用状態に
    for (apr_size_t i = 1; i < ARRAY_SIZE_OF(progress_list_); ++i) {
        if (progress_list_[i].end_time == ENTRY_UNUSED) {
            continue;
        }

        if ((progress_list_[i].end_time != ENTRY_USED) &&
            ((current_time - progress_list_[i].end_time) >
             apr_time_from_sec(PST_PROGRESS_HOLD_SEC))) {
            memset(progress_list_ + i, 0,
                   sizeof(RequestReader::post_progress_t));
        } else if (progress_list_[i].id == upload_id) {
            return progress_list_ + i;
        }
    }

    // 未使用エントリを探して登録
    for (apr_size_t i = 1; i < ARRAY_SIZE_OF(progress_list_); i++) {
        if (progress_list_[i].end_time == ENTRY_UNUSED) {
            progress_list_[i].id            = upload_id;
            progress_list_[i].end_time      = ENTRY_USED;
            progress_list_[i].is_succeeded  = false;

            return (progress_list_ + i);
        }
    }

    // 未使用エントリが無ければダミーを返す
    return progress_list_;
}

PostProgressList *PostProgressList::get_instance(apr_shm_t *shm)
{
    PostProgressList *progress_list;

#ifdef DEBUG
    if (apr_shm_size_get(shm) != get_memory_size()) {
        THROW(MESSAGE_SHM_SIZE_INVALID);
    }
#endif

    progress_list =
        reinterpret_cast<PostProgressList *>(apr_shm_baseaddr_get(shm));
    new(progress_list) PostProgressList;

    return progress_list;
}

apr_size_t PostProgressList::get_memory_size()
{
    return sizeof(PostProgressList);
}

void PostProgressList::dump_list(PostProgressList *progress_list)
{
    for (apr_size_t i = 1; i < DLD_MAX_SESSION_COUNT; i++) {
        if (progress_list->progress_list_[i].end_time == ENTRY_UNUSED) {
            continue;
        }
        dump_progress(progress_list->progress_list_ + i);
    }
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
void PostProgressList::dump_progress(RequestReader::post_progress_t *progress)
{
    cerr << "------------------------------------------------------------" << endl;
    cerr << progress->id << ": ";
    cerr << progress->read_size << "/" << progress->total_size;
    if (progress->end_time != ENTRY_USED) {
        cerr << " (finish)";
    }
    cerr << endl;
}


// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
