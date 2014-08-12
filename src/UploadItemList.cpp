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
 * $Id: UploadItemList.cpp 2756 2007-12-11 10:57:59Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <cstddef>
#include <iostream>
#include <iomanip>

#include "apr_time.h"

#include "UploadItemList.h"
#include "Message.h"
#include "Auxiliary.h"
#include "SourceInfo.h"

using namespace std;

SOURCE_INFO_ADD("$Id: UploadItemList.cpp 2756 2007-12-11 10:57:59Z svn $");

#define AS_UHEADER(pointer)     reinterpret_cast<UploadItem::header_t *>(pointer)
#define AS_UITEM_LIST(pointer)  reinterpret_cast<UploadItemList *>(pointer)

/******************************************************************************
 * public メソッド
 *****************************************************************************/
void UploadItemList::add(UploadItem *uitem)
{
    apr_size_t i;
    apr_size_t header_list_size;

    // MEMO: header_list_ は mtime に基づいて降順に並んでいる

    header_list_size = size();

    i = get_insert_index(uitem);
    if (i == header_list_size) {
        if (header_list_size == max_list_size_) { // 追加する必要なし
            return;
        } else { // 末尾に追加
            load_header(uitem, get_header_list() + header_list_size);
        }
    } else { // 挿入
        memmove(get_header_list() + i + 1, get_header_list() + i,
                sizeof(UploadItem::header_t) * (header_list_size - i));
        load_header(uitem, get_header_list() + i);
    }

    total_file_size_ += uitem->get_file_size();
    header_list_size_++;
}

void UploadItemList::remove(apr_size_t item_id)
{
    apr_size_t i;
    apr_uint64_t file_size;
    const UploadItem::header_t *header;
    apr_size_t header_list_size;

    header_list_size = size();

    if (header_list_size == 0) {
        THROW(MESSAGE_BUG_FOUND);
    }

    i = get_index_by_id(item_id);
    header = get_by_index(i);

    file_size = header->file_size;

    if (i != (header_list_size - 1)) {
        memmove(get_header_list() + i, get_header_list() + i + 1,
                sizeof(UploadItem::header_t) * (header_list_size - i - 1));
    }

    if (total_file_size_ < file_size) {
        THROW(MESSAGE_BUG_FOUND);
    }

    total_file_size_ -= file_size;
    header_list_size_--;
}

UploadItemList *UploadItemList::get_instance(apr_shm_t *shm,
                                             apr_uint64_t max_file_size,
                                             apr_size_t max_list_size)
{
    UploadItemList *uheader_list;

    uheader_list = AS_UITEM_LIST(apr_shm_baseaddr_get(shm));
    new(uheader_list) UploadItemList(shm, max_file_size, max_list_size);

    return uheader_list;
}

UploadItemList *UploadItemList::child_init(apr_shm_t *shm)
{
    return AS_UITEM_LIST(apr_shm_baseaddr_get(shm));
}

apr_size_t UploadItemList::get_memory_size(apr_size_t max_list_size)
{
    return sizeof(UploadItemList) +
        (sizeof(UploadItem::header_t)*max_list_size);
}

void UploadItemList::dump_header_list(apr_pool_t *pool,
                                      UploadItemList *list)
{
    apr_size_t size;

    size = list->size();
    for (apr_size_t i = 0; i < size; i++) {
        dump_header(pool, list->get_by_index(i));
    }
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
UploadItemList::UploadItemList(apr_shm_t *shm, apr_uint64_t max_file_size,
                               apr_size_t max_list_size)
  : header_list_size_(0),
    total_file_size_(0),
    max_file_size_(max_file_size),
    max_list_size_(max_list_size)
{
#ifdef DEBUG
    if (apr_shm_size_get(shm) != get_memory_size(max_list_size)) {
        THROW(MESSAGE_SHM_SIZE_INVALID);
    }
#endif
}

apr_size_t UploadItemList::get_insert_index(UploadItem *uitem) const
{
    apr_size_t i;
    apr_size_t header_list_size;

    header_list_size = size();

    for (i = 0; i < header_list_size; i++) {
        if (uitem->get_mtime() >= get_by_index(i)->mtime) {
            return i;
        }
    }
    return i;
}

apr_size_t UploadItemList::get_index_by_id(apr_size_t item_id) const
{
    apr_size_t header_list_size;

    header_list_size = size();

    // 頻繁には呼ばれ無いはずなので，線形探索...
    for (apr_size_t i = 0; i < header_list_size; i++) {
        if (get_by_index(i)->id == item_id) {
            return i;
        }
    }

    THROW(MESSAGE_LIST_ID_INVALID);
}

void UploadItemList::load_header(UploadItem *uitem,
                                 UploadItem::header_t *header)
{
    *header = *(AS_UHEADER(uitem));
}

void UploadItemList::dump_header(apr_pool_t *pool,
                                 const UploadItem::header_t *header)
{
    cerr << setw(10) << static_cast<unsigned int>(header->id) << " ";
    cout << "[" << header->file_name << "] ";
    cout << "("; dump_date(pool, header->mtime); cout << ")" << endl;
}

void UploadItemList::dump_date(apr_pool_t *pool, apr_time_t time)
{
    apr_time_exp_t time_exp;
    apr_size_t time_str_size;
    char time_str[64];

    check_apr_error(pool, apr_time_exp_lt(&time_exp, time));
    check_apr_error(pool,
                    apr_strftime(time_str, &time_str_size, sizeof(time_str),
                                 "%y/%m/%d %H:%M:%S",
                                 &time_exp));
    cout << time_str;
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_UploadItemList
#include "TestRunner.h"

static const apr_uint64_t TEST_FILE_SIZE    = 1*1024LL*1024LL*1024LL; // 1GB
static const apr_size_t MAX_LIST_SIZE       = 100;
static const apr_uint64_t MAX_FILE_SIZE     = TEST_FILE_SIZE * MAX_LIST_SIZE / 2;

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << endl;
}

UploadItem *create_uitem(apr_pool_t *pool, apr_uint64_t file_size)
{
    return UploadItem::get_instance
        (pool, 0, 0, file_size, 0, "", "", "", "", "", "",
         MESSAGE_TEST_COMMENT, "", false);
}

void run_full(apr_pool_t *pool, UploadItemList *item_list)
{
    UploadItem *uitem;

    show_test_name("full");

    uitem = create_uitem(pool, TEST_FILE_SIZE);

    while (!(item_list->is_full(TEST_FILE_SIZE))) {
        item_list->add(uitem);
    }
    if (item_list->size() != (MAX_FILE_SIZE/TEST_FILE_SIZE)) {
        THROW(MESSAGE_BUG_FOUND);
    }

    while (item_list->size() < MAX_LIST_SIZE) {
        item_list->add(uitem);
    }

    while ((item_list->is_full(TEST_FILE_SIZE))) {
        item_list->remove(uitem->get_id());
    }
    item_list->add(uitem);
    if (item_list->size() != (MAX_FILE_SIZE/TEST_FILE_SIZE)) {
        THROW(MESSAGE_BUG_FOUND);
    }
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    apr_shm_t *item_list_shm;
    UploadItemList *item_list;

    if (argc != 1) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    show_line();

    item_list_shm = create_shm
        (pool, UploadItemList::get_memory_size(MAX_LIST_SIZE));
    item_list = UploadItemList::get_instance
        (item_list_shm, MAX_FILE_SIZE, MAX_LIST_SIZE);

    run_full(pool, item_list);
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
