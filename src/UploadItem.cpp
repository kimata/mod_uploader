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
 * $Id: UploadItem.cpp 2858 2008-04-12 14:25:08Z svn $
 *****************************************************************************/

#include "Environment.h"

#include <memory>
#ifdef DEBUG
#include <iostream>
#include <iomanip>
#endif

#include "UploadItem.h"
#include "TemporaryPool.h"
#include "PostDataChecker.h"
#include "Message.h"
#include "Auxiliary.h"
#include "SourceInfo.h"

SOURCE_INFO_ADD("$Id: UploadItem.cpp 2858 2008-04-12 14:25:08Z svn $");

#define AS_BCOMMENT(pointer)        reinterpret_cast<bbs_comment_t *>(pointer)
#define UINT16(pointer)             static_cast<apr_uint16_t>(pointer)


/******************************************************************************
 * public メソッド
 *****************************************************************************/
UploadItem::UploadItem()
{

}

UploadItem *UploadItem::get_instance(void *memory, apr_size_t item_id,
                                     apr_size_t item_index,
                                     apr_uint64_t file_size, apr_time_t mtime,
                                     const char *ip_address,
                                     const char *file_name,
                                     const char *file_mime,
                                     const char *file_digest,
                                     const char *remove_pass,
                                     const char *download_pass,
                                     const char *comment,
                                     const char *code_pat,
                                     bool is_need_validate)
{
    UploadItem *uitem;
    const char *date;
    const char *file_ext;
    TemporaryPool temp_pool;

    if (is_need_validate) {
        PostDataChecker::validate_uitem(temp_pool.get(), file_size, mtime,
                                        &file_name, &file_mime, file_digest,
                                        remove_pass, download_pass, &comment,
                                        code_pat);
    }

    memset(memory, 0, sizeof(UploadItem));
    uitem = new(memory) UploadItem;

    date = get_formatted_date(temp_pool.get(), mtime);
    file_ext = ::get_file_ext(file_name);

    uitem->header_.id           = item_id;
    uitem->header_.index        = item_index;
    uitem->header_.mtime        = mtime;
    uitem->header_.atime        = mtime;
    uitem->header_.file_size    = file_size;

#ifdef USE_GZIP
    uitem->header_.is_gzipped   = 1;
#endif

    strncpy(uitem->header_.date, date, ITM_MAX_DATE_SIZE);
    strncpy(uitem->header_.ip_address, ip_address, ITM_MAX_IP_ADDRESS_SIZE);
    strncpy(uitem->header_.file_name, file_name, ITM_MAX_FILE_NAME_SIZE);
    strncpy(uitem->header_.file_mime, file_mime, ITM_MAX_FILE_MIME_SIZE);
    strncpy(uitem->header_.file_ext, file_ext, ITM_MAX_FILE_EXT_SIZE);
    strncpy(uitem->header_.file_digest, file_digest, ITM_MAX_FILE_DIGEST_SIZE);
    strncpy(uitem->header_.remove_pass, remove_pass, ITM_MAX_REMOVE_PASS_SIZE);
    strncpy(uitem->header_.download_pass, download_pass, ITM_MAX_DOWNLOAD_PASS_SIZE);
    strncpy(uitem->header_.comment, comment, ITM_MAX_COMMENT_SIZE);

    return uitem;
}

UploadItem *UploadItem::get_instance(apr_pool_t *pool, apr_size_t item_id,
                                     apr_size_t item_index,
                                     apr_uint64_t file_size, apr_time_t mtime,
                                     const char *ip_address,
                                     const char *file_name,
                                     const char *file_mime,
                                     const char *file_digest,
                                     const char *remove_pass,
                                     const char *download_pass,
                                     const char *comment,
                                     const char *code_pat,
                                     bool is_need_validate)
{
    void *memory;

    APR_PCALLOC(memory, void *, pool, sizeof(UploadItem));

    return get_instance(memory, item_id, item_index, file_size, mtime,
                        ip_address, file_name, file_mime, file_digest,
                        remove_pass, download_pass, comment, code_pat,
                        is_need_validate);
}

int UploadItem::cmp_file_name(const UploadItem *a, const UploadItem *b)
{
    return strcmp(b->get_file_name(), a->get_file_name());
}

int UploadItem::cmp_file_size(const UploadItem *a, const UploadItem *b)
{
    apr_uint64_t a_file_size;
    apr_uint64_t b_file_size;

    a_file_size = a->get_file_size();
    b_file_size = b->get_file_size();

    if (b_file_size > a_file_size) {
        return 1;
    } else if (b_file_size == a_file_size) {
        return 0;
    } else {
        return -1;
    }
}

int UploadItem::cmp_file_mime(const UploadItem *a, const UploadItem *b)
{
    return strcmp(b->get_file_mime(), a->get_file_mime());
}

int UploadItem::cmp_file_atime(const UploadItem *a, const UploadItem *b)
{
    apr_time_t a_atime;
    apr_time_t b_atime;

    a_atime = a->get_atime();
    b_atime = b->get_atime();

    if (b_atime > a_atime) {
        return -1;
    } else if (b_atime == a_atime) {
        return 0;
    } else {
        return 1;
    }
}

int UploadItem::cmp_comment(const UploadItem *a, const UploadItem *b)
{
    return strcmp(b->get_comment(), a->get_comment());
}

int UploadItem::cmp_download_count(const UploadItem *a, const UploadItem *b)
{
    apr_size_t a_download_count;
    apr_size_t b_download_count;

    a_download_count = a->get_download_count();
    b_download_count = b->get_download_count();

    if (b_download_count > a_download_count) {
        return 1;
    } else if (b_download_count == a_download_count) {
        return 0;
    } else {
        return -1;
    }
}


/******************************************************************************
 * private メソッド
 *****************************************************************************/
const char *UploadItem::get_formatted_date(apr_pool_t *pool, apr_time_t time)
{
    apr_time_exp_t time_exp;
    char *date;
    apr_size_t size;

    APR_PALLOC(date, char *, pool, ITM_MAX_DATE_SIZE);
    apr_time_exp_tz(&time_exp, time, SYS_TIMEZONE_OFFSET);
    apr_strftime(date, &size, ITM_MAX_DATE_SIZE, ITM_DATE_FORMAT, &time_exp);

    return date;
}


/******************************************************************************
 * テスト
 *****************************************************************************/
#ifdef DEBUG_UploadItem
#include <cstddef>

#include "TestRunner.h"

// cstddef の offsetof は POD じゃないと使えないので，自前で強引に定義
#define OFFSETOF(s, m) \
    ((size_t)((reinterpret_cast<apr_uint64_t>(&(((s *)0x100)->m)) - 0x100) & \
              0xfff))

void show_usage(const char *prog_name)
{
    cerr << "Usage: " << prog_name << endl;
}

void check_item(const char *key, size_t value, size_t expected)
{
    ASSERT_EQUAL(value, expected);
    show_item(key, value);
}

void run_size_32()
{
    show_test_name("size");

    check_item("sizeof(UploadItem)", sizeof(UploadItem), 608);

    show_spacer();

    check_item("sizeof(header_t)", sizeof(UploadItem::header_t), 608);

    check_item("- identifier", OFFSETOF(UploadItem::header_t, identifier), 0);
    check_item("- version", OFFSETOF(UploadItem::header_t, version), 16);
    check_item("- id", OFFSETOF(UploadItem::header_t, id), 24);
    check_item("- index", OFFSETOF(UploadItem::header_t, index), 28);
    check_item("- download_count", OFFSETOF(UploadItem::header_t, download_count), 32);
    check_item("- file_size", OFFSETOF(UploadItem::header_t, file_size), 40);
    check_item("- mtime", OFFSETOF(UploadItem::header_t, mtime), 48);
    check_item("- atime", OFFSETOF(UploadItem::header_t, atime), 56);
    check_item("- date", OFFSETOF(UploadItem::header_t, date), 64);
    check_item("- ip_address", OFFSETOF(UploadItem::header_t, ip_address), 96);
    check_item("- file_name", OFFSETOF(UploadItem::header_t, file_name), 136);
    check_item("- file_mime", OFFSETOF(UploadItem::header_t, file_mime), 200);
    check_item("- file_digest", OFFSETOF(UploadItem::header_t, file_digest), 272);
    check_item("- remove_pass", OFFSETOF(UploadItem::header_t, remove_pass), 320);
    check_item("- download_pass", OFFSETOF(UploadItem::header_t, download_pass), 336);
    check_item("- comment", OFFSETOF(UploadItem::header_t, comment), 352);
}

void run_size_64()
{
    show_test_name("size");

    check_item("sizeof(UploadItem)", sizeof(UploadItem), 616);

    show_spacer();

    check_item("sizeof(header_t)", sizeof(UploadItem::header_t), 616);

    check_item("- identifier", OFFSETOF(UploadItem::header_t, identifier), 0);
    check_item("- version", OFFSETOF(UploadItem::header_t, version), 16);
    check_item("- id", OFFSETOF(UploadItem::header_t, id), 24);
    check_item("- index", OFFSETOF(UploadItem::header_t, index), 32);
    check_item("- download_count", OFFSETOF(UploadItem::header_t, download_count), 40);
    check_item("- file_size", OFFSETOF(UploadItem::header_t, file_size), 48);
    check_item("- mtime", OFFSETOF(UploadItem::header_t, mtime), 56);
    check_item("- atime", OFFSETOF(UploadItem::header_t, atime), 64);
    check_item("- date", OFFSETOF(UploadItem::header_t, date), 72);
    check_item("- ip_address", OFFSETOF(UploadItem::header_t, ip_address), 104);
    check_item("- file_name", OFFSETOF(UploadItem::header_t, file_name), 144);
    check_item("- file_mime", OFFSETOF(UploadItem::header_t, file_mime), 208);
    check_item("- file_digest", OFFSETOF(UploadItem::header_t, file_digest), 280);
    check_item("- remove_pass", OFFSETOF(UploadItem::header_t, remove_pass), 328);
    check_item("- download_pass", OFFSETOF(UploadItem::header_t, download_pass), 344);
    check_item("- comment", OFFSETOF(UploadItem::header_t, comment), 360);
}

void run_size()
{
    if (sizeof(long) == 4) {
        run_size_32();
    } else {
        run_size_64();
    }
}

void run_all(apr_pool_t *pool, int argc, const char * const *argv)
{
    if (argc != 1) {
        THROW(MESSAGE_ARGUMENT_INVALID);
    }

    show_line();
    run_size();
}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-dos
// End:
